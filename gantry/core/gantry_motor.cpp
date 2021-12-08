#include "gantry/core/gantry_motor.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/utils.hpp"

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Ack>
    complete_queue("Complete Queue");

static motor_class::Motor motor{
    interfaces::get_spi(),
    lms::LinearMotionSystemConfig<lms::BeltConfig>{
        .mech_config =
            lms::BeltConfig{.belt_pitch = 2, .pulley_tooth_count = 10},
        .steps_per_rev = 200,
        .microstep = 16},
    interfaces::get_motor_hardware_iface(),
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    utils::register_config(),
    motor_queue,
    complete_queue};

/**
 * Access to the global motor.
 *
 * @return The motor.
 */
auto gantry_motor::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}