#include <stdio.h>

#include <iostream>

#include "catch2/catch.hpp"
#include "pipettes/tests/mock_message_queue.hpp"
#include "pipettes/tests/test_motor_interrupt_handler.hpp"

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
        REQUIRE(handler.has_messages());
        handler.update_move();
        while (handler.can_step()) {
            handler.tick();
        }

        REQUIRE(handler.get_current_position() == temp_steps);
    };
    GIVEN("in 256th micro-stepping mode") {
        // one rotation in 256th microstepping = 51200 steps
        sq32_31 temp_steps = 0xC800LL << 31;
//        sq32_31 temp_steps = 2823ULL << 29;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        handler.update_move();
        while (handler.can_step()) {
            handler.tick();
        }
        REQUIRE(handler.get_current_position() == temp_steps);
    };
}

TEST_CASE("move less than a full rotation") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);

    GIVEN("in full stepping mode") {
        // equivalent of 28.23 steps
        sq32_31 temp_steps = 2823LL << 29;
        Move msg1 = Move{.target_position = temp_steps};
        queue.try_write(msg1);
        REQUIRE(handler.get_current_position() == 0);
        handler.update_move();
        while (handler.can_step()) {
            handler.tick();
//            step_motor(handler);
        }
        REQUIRE(handler.get_current_position() == temp_steps);
    }
}
//  TEST_CASE("move multiple full rotations") {}

// SCENARIO("moving a specific distance in fractional steps") {
//     TEST_CASE("") {
//         GIVEN("") {}
//     }
// }
//
// SCENARIO("overflow positional value") {
//     TEST_CASE("Move in 10mm increments to largest max X position") {
//         // test all of the possible microstepping values here.
//         GIVEN("X microstepping") {}
//         GIVEN("Y microstepping") {}
//     }
// }