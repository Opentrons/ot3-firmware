#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

#define TO_RADIX 31
static constexpr sq0_31 default_velocity = 0x1 << (TO_RADIX - 1);

TEST_CASE("position starts at zero") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    handler.set_current_position(0x0);

    GIVEN("a positive direction") {
        sq32_31 target = 0x1LL << TO_RADIX;
        Move msg1 =
            Move{.target_position = target, .velocity = default_velocity};
        handler.set_buffered_move(msg1);

        THEN("we should take two tick to step one") {
            REQUIRE(!handler.tick());
            REQUIRE(handler.tick());
            AND_THEN("that position should be greater than zero") {
                REQUIRE(handler.get_current_position() > 0x0);
            }
        }
    }
}

TEST_CASE("move less than a full step") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    handler.set_current_position(0x0);

    // equivalent of 5.5 steps
    sq32_31 initial_target = 51LL << (TO_RADIX - 2);
    // equivalent of 0.5 steps
    const sq32_31 fractional_step = 1LL << (TO_RADIX - 2);

    GIVEN("one fractional move") {
        Move msg1 = Move{.target_position = initial_target,
                         .velocity = default_velocity};
        handler.set_buffered_move(msg1);
        THEN("tick the position 12 whole steps") {
            for (int i = 0; i < 12; i++) {
                REQUIRE(!handler.tick());
                REQUIRE(handler.tick());
            }
            AND_THEN(
                "the last tick should return false as it is a fractional "
                "step") {
                REQUIRE(!handler.tick());
                // position should be all the full steps, without the fractional
                // step.
                REQUIRE(handler.get_current_position() ==
                        initial_target - fractional_step);
            }
        }
    }

    GIVEN("two fractional moves") {
        // equivalent of 11 steps
        sq32_31 doubled_target = initial_target << 1;
        const sq32_31 starting_position = initial_target - fractional_step;
        handler.set_current_position(starting_position);

        Move msg1 = Move{.target_position = doubled_target,
                         .velocity = default_velocity};
        handler.set_buffered_move(msg1);

        WHEN(
            "we are on the second move, we should be able to tick at the "
            "end.") {
            REQUIRE(handler.get_current_position() == starting_position);
            THEN(
                "tick the position 13 times, since we are moving an extra "
                "whole step") {
                for (int i = 0; i < 13; i++) {
                    REQUIRE(handler.tick());
                    REQUIRE(!handler.tick());
                }
                AND_THEN(
                    "the current position should be exactly the doubled "
                    "target") {
                    REQUIRE(handler.get_current_position() == doubled_target);
                }
            }
        }
    }
}

TEST_CASE("position starts above zero") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    sq32_31 starting_position = 200LL << TO_RADIX;
    handler.set_current_position(starting_position);
    sq0_31 increment = (1LL << (TO_RADIX - 2));

    GIVEN("Some number of steps in a positive direction") {
        sq32_31 target = starting_position + increment;
        Move msg1 = Move{.target_position = target, .velocity = increment};
        handler.set_buffered_move(msg1);
        handler.tick();
        REQUIRE(handler.get_current_position() == target);
    }

    GIVEN("Some number of steps in a negative direction") {
        increment = -increment;
        sq32_31 target = starting_position + increment;
        sq0_31 velocity_init = increment;
        Move msg1 = Move{.target_position = target, .velocity = velocity_init};
        handler.set_buffered_move(msg1);
        handler.tick();
        REQUIRE(handler.get_current_position() == target);
    }
}

TEST_CASE("moves that result in out of range positions") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue> handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    handler.set_message_queue(&queue);

    GIVEN("Move past the largest possible value for position") {
        // Reaching the max possible position will probably be almost impossible
        // to do in the system. Rather than testing that tick detects this type
        // of overflow, we should just check that the overflow function works
        // correctly for very large numbers.

        sq32_31 position_1 = 0x4000000000000000;
        sq32_31 position_2 = position_1 + position_1;
        REQUIRE(handler.overflow(position_1, position_2));
    }

    GIVEN("Move past the lowest possible value for position (zero)") {
        // Here we should give a negative number
        sq32_31 current_position = 0x0;
        const sq0_31 neg_velocity = -default_velocity;
        handler.set_current_position(current_position);
        const sq32_31 target = 0x7FFFFFF7 << 8;
        Move msg1 = Move{.target_position = target, .velocity = neg_velocity};
        handler.set_buffered_move(msg1);

        THEN("the motor should not pulse, and position should be unchanged.") {
            // (TODO lc): we don't have true error handling yet
            // this should probably return some type of error in
            // the instance of an overflow.
            REQUIRE(!handler.tick());
            REQUIRE(handler.get_current_position() == current_position);
            AND_THEN("the move should be finished") {
                handler.pulse();
                REQUIRE(handler.has_active_move == false);
            }
        }
    }
}
