#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"

void start_motor_handler(
    freertos_message_queue::FreeRTOSMessageQueue<Move>& queue);
void reset_motor_handler();