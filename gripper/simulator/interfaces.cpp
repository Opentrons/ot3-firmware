#include "gripper/core/interfaces.hpp"

#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/simulation/spi.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * The SPI bus.
 */
static auto spi_comms = sim_spi::SimSpiDeviceBase();

/**
 * The motor interface.
 */
static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface();

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::BeltConfig>{
        .mech_config =
            lms::BeltConfig{.belt_pitch = 2, .pulley_tooth_count = 10},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    utils::driver_config(),
    motor_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gripper_tasks::get_queues(), motor_interface);

static motor_interrupt_driver::MotorInterruptDriver A(motor_queue,
                                                      motor_interrupt);

void interfaces::initialize() {}

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::SpiDeviceBase& { return spi_comms; }

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::MotorHardwareIface& {
    return motor_interface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}