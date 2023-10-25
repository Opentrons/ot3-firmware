#include <signal.h>

#include <iostream>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "boost/program_options.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "common/simulation/state_manager.hpp"
#include "eeprom/simulation/eeprom.hpp"
#include "head/core/queues.hpp"
#include "head/core/tasks_proto.hpp"
#include "head/core/utils.hpp"
#include "head/simulator/presence_sensing.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2130_driver.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "spi/simulation/spi.hpp"
#include "task.h"

namespace po = boost::program_options;

/**
 * The SPI busses.
 */
static auto spi_comms_right = spi::hardware::SimSpiDeviceBase();
static auto spi_comms_left = spi::hardware::SimSpiDeviceBase();

/**
 * The motor interfaces.
 */
static auto motor_interface_right =
    sim_motor_hardware_iface::SimMotorHardwareIface(MoveMessageHardware::z_r);
static auto motor_interface_left =
    sim_motor_hardware_iface::SimMotorHardwareIface(MoveMessageHardware::z_l);

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue_right("PQueue Right");

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue_left("PQueue Left");

static tmc2130::configs::TMC2130DriverConfig MotorDriverConfigurations{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 0x2,
                           .run_current = 0x10,
                           .hold_current_delay = 0x7},
            .tpowerdown = {},
            .tcoolthrs = {.threshold = 0},
            .thigh = {.threshold = 0xFFFFF},
            .chopconf = {.toff = 0x5,
                         .hstrt = 0x5,
                         .hend = 0x3,
                         .tbl = 0x2,
                         .mres = 0x4},
            .coolconf = {.sgt = 0x6},
        },
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = 0,
        .GPIO_handle = 0,
    }};

static auto linear_config = lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
    .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12.0,
                                        .gear_reduction_ratio = 4.0},
    .steps_per_rev = 200.0,
    .microstep = 32.0,
    .encoder_pulses_per_rev = 1024.0};

static stall_check::StallCheck stallcheck_right(
    linear_config.get_encoder_pulses_per_mm() / 1000.0F,
    linear_config.get_usteps_per_mm() / 1000.0F, utils::STALL_THRESHOLD_UM);

static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(), head_tasks::get_right_queues(), motor_interface_right,
    stallcheck_right, update_position_queue_right);

static stall_check::StallCheck stallcheck_left(
    linear_config.get_encoder_pulses_per_mm() / 1000.0F,
    linear_config.get_usteps_per_mm() / 1000.0F, utils::STALL_THRESHOLD_UM);

static auto motor_sys_config =
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 32,
        .encoder_pulses_per_rev = 1000};

static motor_class::Motor motor_right{
    motor_sys_config, motor_interface_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_right, update_position_queue_right};

static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(), head_tasks::get_left_queues(), motor_interface_left,
    stallcheck_left, update_position_queue_left);

static motor_class::Motor motor_left{
    motor_sys_config, motor_interface_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_left, update_position_queue_left};

static motor_interrupt_driver::MotorInterruptDriver sim_interrupt_right(
    motor_queue_right, motor_interrupt_right, motor_interface_right,
    update_position_queue_right);
static motor_interrupt_driver::MotorInterruptDriver sim_interrupt_left(
    motor_queue_left, motor_interrupt_left, motor_interface_left,
    update_position_queue_left);

static auto presence_sense_driver =
    presence_sensing_driver::PresenceSensingSimulator();

static std::shared_ptr<state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>>
    state_manager_connection;

static auto state_manager_task = state_manager::StateManagerTask<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto state_manager_task_control =
    freertos_task::FreeRTOSTask<512, decltype(state_manager_task)>{
        state_manager_task};

static auto rmh_tsk =
    motor_hardware_task::MotorHardwareTask{&motor_interface_right, "rmh task"};
static auto lmh_tsk =
    motor_hardware_task::MotorHardwareTask{&motor_interface_left, "lmh task"};

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

auto handle_options(int argc, char** argv) -> po::variables_map {
    auto cmdlinedesc = po::options_description("simulator for the OT-3 head");
    auto envdesc = po::options_description("");
    cmdlinedesc.add_options()("help,h", "Show this help message.");
    auto can_arg_xform = can::sim::transport::add_options(cmdlinedesc, envdesc);
    auto state_mgr_arg_xform = state_manager::add_options(cmdlinedesc, envdesc);
    auto eeprom_arg_xform =
        eeprom::simulator::EEProm::add_options(cmdlinedesc, envdesc);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cmdlinedesc), vm);
    if (vm.count("help")) {
        std::cout << cmdlinedesc << std::endl;
        std::exit(0);
    }
    po::store(po::parse_environment(envdesc, can_arg_xform), vm);
    po::store(po::parse_environment(envdesc, state_mgr_arg_xform), vm);
    po::store(po::parse_environment(envdesc, eeprom_arg_xform), vm);
    po::notify(vm);
    return vm;
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);

    LOG_INIT("HEAD", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    auto options = handle_options(argc, argv);

    state_manager_connection = state_manager::create<
        freertos_synchronization::FreeRTOSCriticalSection>(options);
    state_manager_task_control.start(5, "state mgr task",
                                     &state_manager_connection);

    motor_interface_right.provide_state_manager(state_manager_connection);
    motor_interface_left.provide_state_manager(state_manager_connection);

    motor_interface_right.provide_mech_config(motor_sys_config);
    motor_interface_left.provide_mech_config(motor_sys_config);

    auto canbus = std::make_shared<can::sim::bus::SimCANBus>(
        can::sim::transport::create(options));

    const uint32_t TEMPORARY_SERIAL = 0x103321;
    auto sim_eeprom =
        std::make_shared<eeprom::simulator::EEProm>(options, TEMPORARY_SERIAL);
    auto i2c_device_map = i2c::hardware::SimI2C::DeviceMap{
        {sim_eeprom->get_address(), *sim_eeprom}};
    auto i2c3 = std::make_shared<i2c::hardware::SimI2C>(i2c_device_map);

    head_tasks::start_tasks(
        *canbus, motor_left.motion_controller, motor_right.motion_controller,
        presence_sense_driver, spi_comms_right, spi_comms_left,
        MotorDriverConfigurations, MotorDriverConfigurations, rmh_tsk, lmh_tsk,
        *i2c3, *sim_eeprom);

    vTaskStartScheduler();
    return 0;
}
