#include "motor-control/core/step_motor.hpp"

#include "common/firmware/timer_interrupt.h"
#include "motor-control/core/motor_messages.hpp"
#include "stm32l5xx_hal.h"

using namespace motor_messages;

static auto handler_class = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue,
    freertos_message_queue::FreeRTOSMessageQueue>();

void step_motor() {
    if (handler_class.pulse()) {
        if (handler_class.set_direction_pin(get_direction_bit())) {
            turn_on_direction_pin();  // TODO (AA, 11/1/2021): make sure the DIR
        } else {
            turn_off_direction_pin();
        }
        turn_on_step_pin();
    }
    turn_off_step_pin();
}

void start_motor_handler(
    freertos_message_queue::FreeRTOSMessageQueue<Move>* queue,
    freertos_message_queue::FreeRTOSMessageQueue<Ack>* completed_queue) {
    handler_class.set_message_queue(queue, completed_queue);
}

void reset_motor_handler() { handler_class.reset(); }