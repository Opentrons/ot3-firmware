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
#include "common/core/logging.h"
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
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hdc3020.hpp"
#include "sensors/simulation/mmr920C04.hpp"
#include "sensors/simulation/mock_hardware.hpp"
#include "spi/simulation/spi.hpp"
#include "task.h"

constexpr auto PIPETTE_TYPE = get_pipette_type();

namespace po = boost::program_options;

static spi::hardware::SimSpiDeviceBase spi_comms{};

static auto motor_config = motor_configs::motor_configurations<PIPETTE_TYPE>();

static auto interrupt_queues = interfaces::get_interrupt_queues<PIPETTE_TYPE>();

static auto linear_motor_hardware =
    interfaces::linear_motor::get_motor_hardware();
static auto plunger_interrupt = interfaces::linear_motor::get_interrupt(
    linear_motor_hardware, interrupt_queues.plunger_queue);
static auto plunger_interrupt_driver =
    interfaces::linear_motor::get_interrupt_driver(
        linear_motor_hardware, interrupt_queues.plunger_queue,
        plunger_interrupt);
static auto linear_motion_control =
    interfaces::linear_motor::get_motion_control(linear_motor_hardware,
                                                 interrupt_queues);

static auto gear_hardware =
    interfaces::gear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto gear_interrupts =
    interfaces::gear_motor::get_interrupts(gear_hardware, interrupt_queues);
static auto gear_motion_control =
    interfaces::gear_motor::get_motion_control(gear_hardware, interrupt_queues);

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

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::HighThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::GearMotionControl& gear_motion,
    test_mocks::MockSensorHardware& fake_sensor_hw,
    eeprom::simulator::EEProm& sim_eeprom) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              fake_sensor_hw, id, sim_eeprom);

    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);

    // TODO Convert gear motor tasks
    gear_motor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  gear_motion,
                                  peripheral_tasks::get_spi_client(), conf, id);
}

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::LowThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::UnavailableGearMotionControl&,
    test_mocks::MockSensorHardware& fake_sensor_hw,
    eeprom::simulator::EEProm& sim_eeprom) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              fake_sensor_hw, id, sim_eeprom);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
}

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc = po::options_description("simulator for OT-3 pipettes");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    cmdlinedesc.add_options()(
        "mount,m", po::value<std::string>()->default_value("left"),
        "Which mount ('right' or 'left') to attach to. May be specified in an "
        "environment variable called MOUNT.");
    envdesc.add_options()("MOUNT",
                          po::value<std::string>()->default_value("left"));
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(
                  envdesc,
                  [can_arg_xform, eeprom_arg_xform](
                      const std::string& input_val) -> std::string {
                      if (input_val == "MOUNT") {
                          return "mount";
                      };
                      auto can_xformed = can_arg_xform(input_val);
                      if (can_xformed != "") {
                          return can_xformed;
                      }
                      auto eeprom_xformed = eeprom_arg_xform(input_val);
                      return eeprom_xformed;
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

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    LOG_INIT(PipetteTypeString[PIPETTE_TYPE], []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    const uint32_t TEMPORARY_PIPETTE_SERIAL =
        temporary_serial_number(PIPETTE_TYPE);
    auto options = handle_options(argc, argv);
    auto hdcsensor = std::make_shared<hdc3020_simulator::HDC3020>();
    auto capsensor = std::make_shared<fdc1004_simulator::FDC1004>();
    auto sim_eeprom = std::make_shared<eeprom::simulator::EEProm>(
        options, TEMPORARY_PIPETTE_SERIAL);
    auto fake_sensor_hw = std::make_shared<test_mocks::MockSensorHardware>();
    auto pressuresensor =
        std::make_shared<mmr920C04_simulator::MMR920C04>(*fake_sensor_hw);
    i2c::hardware::SimI2C::DeviceMap sensor_map_i2c1 = {
        {hdcsensor->get_address(), *hdcsensor},
        {capsensor->get_address(), *capsensor},
        {pressuresensor->get_address(), *pressuresensor}};

    i2c::hardware::SimI2C::DeviceMap sensor_map_i2c3 = {
        {sim_eeprom->get_address(), *sim_eeprom}};
    auto i2c3_comms = std::make_shared<i2c::hardware::SimI2C>(sensor_map_i2c3);

    auto i2c1_comms = std::make_shared<i2c::hardware::SimI2C>(sensor_map_i2c1);
    auto can_bus_1 = std::make_shared<can::sim::bus::SimCANBus>(
        can::sim::transport::create(options));
    auto node = node_from_options(options);
    central_tasks::start_tasks(*can_bus_1, node);
    peripheral_tasks::start_tasks(*i2c3_comms, *i2c1_comms, spi_comms);
    initialize_motor_tasks(node, motor_config.driver_configs,
                           gear_motion_control, *fake_sensor_hw, *sim_eeprom);

    vTaskStartScheduler();
}
