#include "gantry/simulator/interfaces.hpp"

#include <iostream>
#include <memory>
#include <string>

#include "boost/program_options.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"
#include "common/simulation/state_manager.hpp"
#include "gantry/core/axis_type.h"
#include "gantry/core/interfaces_proto.hpp"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "spi/simulation/spi.hpp"

namespace po = boost::program_options;
/**
 * The SPI bus.
 */
static auto spi_comms = spi::hardware::SimSpiDeviceBase();

const uint32_t TEMPORARY_SERIAL = 0x103321;
std::shared_ptr<eeprom::simulator::EEProm> sim_eeprom;
std::shared_ptr<i2c::hardware::SimI2C> i2c2;
/**
 * The motor interface.
 */
static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface(
    get_axis_type() == GantryAxisType::gantry_x ? MoveMessageHardware::x
                                                : MoveMessageHardware::y);

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue("Position Queue");

static tmc2130::configs::TMC2130DriverConfig driver_configs{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 0x2,
                                 .run_current = 0x18,
                                 .hold_current_delay = 0x7},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x5,
                               .hstrt = 0x5,
                               .hend = 0x3,
                               .tbl = 0x2,
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = 0,
        .GPIO_handle = 0,
    }};

static auto linear_motion_sys_config = utils::linear_motion_system_config();

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    linear_motion_sys_config, motor_interface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue, update_position_queue};

static stall_check::StallCheck stallcheck(
    utils::linear_motion_system_config().get_encoder_pulses_per_mm() / 1000.0F,
    utils::linear_motion_system_config().get_usteps_per_mm() / 1000.0F,
    utils::STALL_THRESHOLD_UM);

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gantry::queues::get_queues(), motor_interface, stallcheck,
    update_position_queue);

static motor_interrupt_driver::MotorInterruptDriver A(motor_queue,
                                                      motor_interrupt,
                                                      motor_interface,
                                                      update_position_queue);
void interfaces::initialize() {}

static po::variables_map options{};

static std::shared_ptr<state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>>
    state_manager_connection;

static auto state_manager_task = state_manager::StateManagerTask<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto state_manager_task_control =
    freertos_task::FreeRTOSTask<512, decltype(state_manager_task)>{
        state_manager_task};

void interfaces::initialize_sim(int argc, char** argv) {
    auto cmdlinedesc = po::options_description(
        std::string("simulator for the OT-3 gantry ") +
        ((get_axis_type() == gantry_x) ? "X" : "Y") + " axis");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto state_mgr_arg_xform = state_manager::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);

    po::store(po::parse_command_line(argc, argv, cmdlinedesc), options);
    if (options.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(envdesc, can_arg_xform), options);
    po::store(po::parse_environment(envdesc, eeprom_arg_xform), options);
    po::store(po::parse_environment(envdesc, state_mgr_arg_xform), options);
    po::notify(options);

    state_manager_connection = state_manager::create<
        freertos_synchronization::FreeRTOSCriticalSection>(options);
    state_manager_task_control.start(5, "state mgr task",
                                     &state_manager_connection);
    motor_interface.provide_state_manager(state_manager_connection);
    motor_interface.provide_mech_config(linear_motion_sys_config);
    sim_eeprom =
        std::make_shared<eeprom::simulator::EEProm>(options, TEMPORARY_SERIAL);
    auto i2c_device_map = i2c::hardware::SimI2C::DeviceMap{
        {sim_eeprom->get_address(), *sim_eeprom}};
    i2c2 = std::make_shared<i2c::hardware::SimI2C>(i2c_device_map);
}

std::shared_ptr<can::bus::CanBus> canbus;

auto interfaces::get_can_bus() -> can::bus::CanBus& {
    canbus.reset(
        new can::sim::bus::SimCANBus(can::sim::transport::create(options)));
    return *canbus;
}

auto interfaces::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::StepperMotorHardwareIface& {
    return motor_interface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}

auto interfaces::get_driver_config() -> tmc2130::configs::TMC2130DriverConfig& {
    return driver_configs;
}

static auto mh_tsk = motor_hardware_task::MotorHardwareTask{
    &motor_interface, "motor hardware task"};
auto interfaces::get_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask& {
    return mh_tsk;
}

auto interfaces::get_sim_eeprom()
    -> std::shared_ptr<eeprom::simulator::EEProm> {
    return sim_eeprom;
}

auto interfaces::get_sim_i2c2() -> std::shared_ptr<i2c::hardware::SimI2C> {
    return i2c2;
}