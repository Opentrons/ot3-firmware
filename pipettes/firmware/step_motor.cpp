#include "motor-control/core/step_motor.hpp"

#include "common/firmware/timer_interrupt.h"
#include "motor-control/core/motor_messages.hpp"

using namespace motor_messages;

static auto handler_class = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue>();

void step_motor() {
    if (handler_class.can_step() && handler_class.tick()) {
        turn_on_step_pin();
    } else {
        if (handler_class.has_messages()) {
            handler_class.update_move();
        } else {
            handler_class.finish_current_move();
        }
    }
    turn_off_step_pin();
}

void start_motor_handler(
    freertos_message_queue::FreeRTOSMessageQueue<Move>& queue) {
    handler_class.set_message_queue(&queue);
}

void reset_motor_handler() { handler_class.reset(); }