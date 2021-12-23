#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "common/simulation/spi.hpp"
#include "head/core/tasks.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "task.h"

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * The SPI busses.
 */
static auto spi_comms_right = sim_spi::SimTMC2130Spi();
static auto spi_comms_left = sim_spi::SimTMC2130Spi();

/**
 * The motor interfaces.
 */
static auto motor_interface_right =
    sim_motor_hardware_iface::SimMotorHardwareIface();
static auto motor_interface_left =
    sim_motor_hardware_iface::SimMotorHardwareIface();

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

motor_driver_config::RegisterConfig MotorDriverConfigurations{
    .gconf = 0x04,
    .ihold_irun = 0x70202,
    .chopconf = 0x40101D5,
    .thigh = 0xFFFFF,
    .coolconf = 0x60000};

static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(), motor_interface_right);

static motor_class::Motor motor_right{
    spi_comms_right,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_right};

static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(), motor_interface_left);

static motor_class::Motor motor_left{
    spi_comms_left,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_left};

int main() {
    head_tasks::start_tasks(canbus, motor_left.motion_controller,
                            motor_left.driver, motor_right.motion_controller,
                            motor_right.driver);

    vTaskStartScheduler();
    return 0;
}