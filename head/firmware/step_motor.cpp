#include "motor-control/core/step_motor.hpp"

#include "common/firmware/timer_interrupt.h"
#include "motor-control/core/motor_messages.hpp"

using namespace motor_messages;

static auto handler_class = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue,
    freertos_message_queue::FreeRTOSMessageQueue>();

void step_motor() {
    if (handler_class.pulse()) {
        if (handler_class.set_direction_pin()) {
            turn_on_direction_pin();
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

MoveStatus get_move_status() {
    auto buffered_move = handler_class.get_buffered_move();
    MoveStatus msg = {.group_id = buffered_move.group_id,
                      .seq_id = buffered_move.seq_id,
                      .remaining_duration = handler_class.get_remaining_ticks(),
                      .current_position = handler_class.get_current_position()};
    return msg;
}
