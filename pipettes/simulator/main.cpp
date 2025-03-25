#include <signal.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "FreeRTOS.h"
#include "boost/program_options.hpp"
#include "can/core/ids.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "common/simulation/state_manager.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/simulator/interfaces.hpp"
#include "sensors/core/mmr920.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hdc3020.hpp"
#include "sensors/simulation/mmr920.hpp"
#include "sensors/simulation/mock_hardware.hpp"
#include "spi/simulation/spi.hpp"
#include "task.h"

constexpr auto PIPETTE_TYPE = get_pipette_type();

namespace po = boost::program_options;

static spi::hardware::SimSpiDeviceBase spi_comms{};

static auto motor_config = motor_configs::motor_configurations<PIPETTE_TYPE>();

static auto interrupt_queues = interfaces::get_interrupt_queues<PIPETTE_TYPE>();

static auto linear_stall_check = stall_check::StallCheck(
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE)
            .get_encoder_pulses_per_mm() /
        1000.0F,
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE)
            .get_usteps_per_mm() /
        1000.0F,
    configs::STALL_THRESHOLD_UM);

// Gear motors have no encoders
static auto gear_stall_check = interfaces::gear_motor::GearStallCheck{
    .left = stall_check::StallCheck(0, 0, 0),
    .right = stall_check::StallCheck(0, 0, 0)};

static auto linear_motor_hardware =
    interfaces::linear_motor::get_motor_hardware();
static auto plunger_interrupt = interfaces::linear_motor::get_interrupt(
    linear_motor_hardware, interrupt_queues, linear_stall_check);
static auto plunger_interrupt_driver =
    interfaces::linear_motor::get_interrupt_driver(
        linear_motor_hardware, interrupt_queues, plunger_interrupt);
static auto linear_motion_control =
    interfaces::linear_motor::get_motion_control(linear_motor_hardware,
                                                 interrupt_queues);

static auto gear_hardware =
    interfaces::gear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto gear_interrupts = interfaces::gear_motor::get_interrupts(
    gear_hardware, interrupt_queues, gear_stall_check);
static auto gear_motion_control =
    interfaces::gear_motor::get_motion_control(gear_hardware, interrupt_queues);

static std::shared_ptr<state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>>
    state_manager_connection;

static auto state_manager_task = state_manager::StateManagerTask<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto state_manager_task_control =
    freertos_task::FreeRTOSTask<512, decltype(state_manager_task)>{
        state_manager_task};

static auto node_from_options(const po::variables_map& options)
    -> can::ids::NodeId {
    auto side = options["mount"].as<std::string>();
    if (side == "left") {
        LOG("On left mount");
        return can::ids::NodeId::pipette_left;
    } else if (side == "right") {
        LOG("On right mount from env var");
        return can::ids::NodeId::pipette_right;
    } else {
        LOG("On left mount from invalid option %s", side.c_str());
        return can::ids::NodeId::pipette_left;
    }
}

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

static const char* PipetteTypeString[] = {
    "SINGLE CHANNEL PIPETTE", "EIGHT CHANNEL PIPETTE",
    "NINETY SIX CHANNEL PIPETTE", "THREE EIGHTY FOUR CHANNEL PIPETTE"};

static auto& sensor_queue_client = sensor_tasks::get_queues();

static auto tail_accessor =
    eeprom::dev_data::DevDataTailAccessor{sensor_queue_client};

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::HighThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::GearMotionControl& gear_motion,
    sim_mocks::MockSensorHardware& fake_sensor_hw_primary,
    sim_mocks::MockSensorHardware& fake_sensor_hw_secondary,
    sensors::hardware::SensorHardwareVersionSingleton& version_wrapper,
    eeprom::simulator::EEProm& sim_eeprom,
    motor_hardware_task::MotorHardwareTask& lmh_tsk,
    interfaces::gear_motor::GearMotorHardwareTasks& gmh_tsks) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c3_poller_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              fake_sensor_hw_primary, fake_sensor_hw_secondary,
                              version_wrapper, id, sim_eeprom, tail_accessor);

    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id, lmh_tsk,
        tail_accessor);

    // TODO Convert gear motor tasks
    gear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, gear_motion,
        peripheral_tasks::get_spi_client(), conf, id, gmh_tsks, tail_accessor);
}

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::LowThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::UnavailableGearMotionControl&,
    sim_mocks::MockSensorHardware& fake_sensor_hw_primary,
    sim_mocks::MockSensorHardware& fake_sensor_hw_secondary,
    sensors::hardware::SensorHardwareVersionSingleton& version_wrapper,
    eeprom::simulator::EEProm& sim_eeprom,
    motor_hardware_task::MotorHardwareTask& lmh_tsk,
    interfaces::gear_motor::UnavailableGearHardwareTasks&) {
    if (get_pipette_type() == EIGHT_CHANNEL) {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_client(),
                                  peripheral_tasks::get_i2c3_poller_client(),
                                  peripheral_tasks::get_i2c1_client(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  fake_sensor_hw_primary,
                                  fake_sensor_hw_secondary, version_wrapper, id,
                                  sim_eeprom, tail_accessor);

    } else /* single channel */ {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_client(),
                                  peripheral_tasks::get_i2c3_poller_client(),
                                  peripheral_tasks::get_i2c1_client(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  fake_sensor_hw_primary, version_wrapper, id,
                                  sim_eeprom, tail_accessor);
    }
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id, lmh_tsk,
        tail_accessor);
}

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc = po::options_description("simulator for OT-3 pipettes");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    cmdlinedesc.add_options()(
        "mount,m", po::value<std::string>()->default_value("left"),
        "Which mount ('right' or 'left') to attach to. May be specified in an "
        "environment variable called MOUNT.");
    envdesc.add_options()("mount",
                          po::value<std::string>()->default_value("left"));
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);
    auto state_mgr_arg_xform = state_manager::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform, eeprom_arg_xform, state_mgr_arg_xform](
                      const std::string& input_val) -> std::string {
                      if (input_val == "MOUNT") {
                          return "mount";
                      }
                      auto can_xformed = can_arg_xform(input_val);
                      if (can_xformed != "") {
                          return can_xformed;
                      }
                      auto eeprom_xformed = eeprom_arg_xform(input_val);
                      if (eeprom_xformed != "") {
                          return eeprom_xformed;
                      }
                      auto state_mgr_xformed = state_mgr_arg_xform(input_val);
                      return state_mgr_xformed;
                  }),
              vm);
    po::notify(vm);
    return vm;
}

uint32_t temporary_serial_number(const PipetteType pipette_type) {
    // TODO there is no pipette name for the ninety six or three eighty four
    // channel pipette. For now they will come out as 'p50s and p50m' in
    // the simulator
    switch (pipette_type) {
        case EIGHT_CHANNEL:
            // Binary value equivalent to P1KMV3120200304A1
            return 0x011F089D;
        case NINETY_SIX_CHANNEL:
            return 0x021F089D;
        case THREE_EIGHTY_FOUR_CHANNEL:
            return 0x031F089D;
        case SINGLE_CHANNEL:
        default:
            // Binary value equivalent to P1KSV3120200304A1
            return 0x001F089D;
    }
}

[[maybe_unused]] static auto provide_state(
    interfaces::gear_motor::UnavailableGearHardware& hardware,
    sim_motor_hardware_iface::StateManagerHandle state_manager_connection)
    -> void {
    // Do nothing for unavailable gear motor
    static_cast<void>(hardware);
    static_cast<void>(state_manager_connection);
}

[[maybe_unused]] static auto provide_state(
    interfaces::gear_motor::GearHardware& hardware,
    sim_motor_hardware_iface::StateManagerHandle state_manager_connection)
    -> void {
    hardware.left.provide_state_manager(state_manager_connection);
    hardware.right.provide_state_manager(state_manager_connection);
    hardware.left.provide_mech_config(configs::linear_motion_sys_config_by_axis(
        PipetteType::NINETY_SIX_CHANNEL));
    hardware.right.provide_mech_config(
        configs::linear_motion_sys_config_by_axis(
            PipetteType::NINETY_SIX_CHANNEL));
}

static auto lmh_tsk = motor_hardware_task::MotorHardwareTask{
    &linear_motor_hardware, "linear motor hardware task"};
static auto gmh_tsks =
    interfaces::gear_motor::get_motor_hardware_tasks(gear_hardware);

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    LOG_INIT(PipetteTypeString[PIPETTE_TYPE], []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    const uint32_t TEMPORARY_PIPETTE_SERIAL =
        temporary_serial_number(PIPETTE_TYPE);
    auto options = handle_options(argc, argv);

    auto node = node_from_options(options);

    state_manager_connection = state_manager::create<
        freertos_synchronization::FreeRTOSCriticalSection>(options);
    state_manager_task_control.start(5, "state mgr task",
                                     &state_manager_connection);

    linear_motor_hardware.change_hardware_id(
        node == can::ids::NodeId::pipette_left ? MoveMessageHardware::z_l
                                               : MoveMessageHardware::z_r);
    linear_motor_hardware.provide_state_manager(state_manager_connection);
    linear_motor_hardware.provide_mech_config(
        linear_motion_control.get_mechanical_config());
    provide_state(gear_hardware, state_manager_connection);

    auto eeprom_model =
        get_pipette_type() == NINETY_SIX_CHANNEL
            ? eeprom::hardware_iface::EEPromChipType::ST_M24128_BF
            : eeprom::hardware_iface::EEPromChipType::ST_M24128_DF;

    auto hdcsensor = std::make_shared<hdc3020_simulator::HDC3020>();
    auto capsensor_front = std::make_shared<fdc1004_simulator::FDC1004>();
    auto capsensor_rear = std::make_shared<fdc1004_simulator::FDC1004>();
    auto sim_eeprom = std::make_shared<eeprom::simulator::EEProm>(
        eeprom_model, options, TEMPORARY_PIPETTE_SERIAL);
    auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    auto fake_sensor_hw_primary =
        std::make_shared<sim_mocks::MockSensorHardware>(version_wrapper,
                                                        sync_control);
    fake_sensor_hw_primary->provide_state_manager(state_manager_connection);
    auto fake_sensor_hw_secondary =
        std::make_shared<sim_mocks::MockSensorHardware>(version_wrapper,
                                                        sync_control);
    fake_sensor_hw_secondary->provide_state_manager(state_manager_connection);
    auto pressuresensor_i2c1 = std::make_shared<mmr920_simulator::MMR920>();
    auto pressuresensor_i2c3 = std::make_shared<mmr920_simulator::MMR920>();
    i2c::hardware::SimI2C::DeviceMap sensor_map_i2c1 = {
        {hdcsensor->get_address(), *hdcsensor},
        {pressuresensor_i2c1->get_address(), *pressuresensor_i2c1}};

    i2c::hardware::SimI2C::DeviceMap sensor_map_i2c3 = {
        {pressuresensor_i2c3->get_address(), *pressuresensor_i2c3},
        {capsensor_rear->get_address(), *capsensor_rear}};

    if (get_pipette_type() == NINETY_SIX_CHANNEL) {
        // On 96 channel, there's an address conflict.
        sensor_map_i2c3.insert({sim_eeprom->get_address(), *sim_eeprom});
        sensor_map_i2c1.insert(
            {capsensor_front->get_address(), *capsensor_front});
    } else {
        sensor_map_i2c1.insert({sim_eeprom->get_address(), *sim_eeprom});
    }
    auto i2c3_comms = std::make_shared<i2c::hardware::SimI2C>(sensor_map_i2c3);

    auto i2c1_comms = std::make_shared<i2c::hardware::SimI2C>(sensor_map_i2c1);
    auto can_bus_1 = std::make_shared<can::sim::bus::SimCANBus>(
        can::sim::transport::create(options));
    central_tasks::start_tasks(*can_bus_1, node);
    peripheral_tasks::start_tasks(*i2c3_comms, *i2c1_comms, spi_comms);
    initialize_motor_tasks(node, motor_config.driver_configs,
                           gear_motion_control, *fake_sensor_hw_primary,
                           *fake_sensor_hw_secondary, version_wrapper,
                           *sim_eeprom, lmh_tsk, gmh_tsks);

    vTaskStartScheduler();
}
