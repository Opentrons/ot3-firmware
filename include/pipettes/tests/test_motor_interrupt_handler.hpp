#include <stdio.h>

#include <iostream>

#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

void step_motor(auto handler);

void step_motor(auto handler) {
    if (handler.can_step() && handler.tick()) {
        printf("ticking");
    } else {
        if (handler.has_messages()) {
            handler.update_move();
        } else {
            handler.finish_current_move();
        }
    }
}