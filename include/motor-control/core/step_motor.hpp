#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"

void start_motor_handler(
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>* queue,
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Ack>*
        completed_queue);
void reset_motor_handler();
motor_messages::MoveStatus get_move_status();