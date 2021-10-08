#include <stdio.h>

#include <iostream>

#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

TEST_CASE("move a full rotation") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);
    handler.reset();

    GIVEN("in full step mode") {
        // one rotation in full stepping = 200 steps
        sq32_31 temp_steps = 0xC8LL << 31;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move) {
                running = false;
            }
        }
        REQUIRE(handler.get_current_position() == temp_steps);
    };
    GIVEN("in 256th micro-stepping mode") {
        // one rotation in 256th microstepping = 51200 steps
        sq32_31 temp_steps = 0xC800LL << 31;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move) {
                running = false;
            }
        }
        REQUIRE(handler.get_current_position() == temp_steps);
    };
}

TEST_CASE("move less than a full step") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);

    GIVEN("one fractional move") {
        // equivalent of 28.23 steps
        sq32_31 temp_steps = 2823LL << 29;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move) {
                running = false;
            }
        }
        REQUIRE(handler.get_current_position() == temp_steps);
    }

    GIVEN("two fractional moves") {
        // equivalent of 28.23 steps
        sq32_31 temp_steps = 2823LL << 29;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move && !handler.has_messages()) {
                running = false;
            }
        }
        REQUIRE(handler.get_current_position() == temp_steps + temp_steps);
    }
}

TEST_CASE("move multiple full rotations in a row") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);

    GIVEN("Movement in incrememnts of 200 steps") {
        sq32_31 temp_steps = 0xC8LL << 31;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        queue.try_write(msg1);
        queue.try_write(msg1);
        queue.try_write(msg1);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move && !handler.has_messages()) {
                running = false;
            }
        }
        REQUIRE(handler.get_current_position() == temp_steps << 2);
    }
}

TEST_CASE("overflow position") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);

    GIVEN("Move past the largest possible value for position") {
        sq32_31 max_position = 0x7FFFFFF76B48C000;
        handler.set_current_position(max_position);
        const sq32_31 temp_steps = 0x7FFFFFF7 << 2;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        bool running = true;
        while (running) {
            handler.pulse();
            if (!handler.has_active_move && !handler.has_messages()) {
                running = false;
            }
        }
        sq32_31 new_position = max_position + temp_steps;
        sq32_31 difference = new_position - handler.get_current_position();
        // Since we've overflowed our position tracker, we would expect it
        // to be a negative number. Once we handle overflow properly, this
        // test should be modified.
        REQUIRE(difference < 1);
    }
}
