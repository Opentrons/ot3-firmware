#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

#define TO_RADIX 31
static constexpr sq0_31 default_velocity =
    0x1 << (TO_RADIX - 1);  // half a step per tick

TEST_CASE("Constant velocity") {
    GIVEN("a duration of 1 tick and velocity at half a step per tick") {
        MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                              mock_message_queue::MockMessageQueue>
            handler{};
        handler.set_current_position(0x0);
        WHEN("moving at half a step per tick") {
            Move msg1 = Move{.duration = 1, .velocity = default_velocity};
            handler.set_buffered_move(msg1);
            THEN("motor would not step in the 1st tick") {
                REQUIRE(!handler.tick());
                AND_THEN("the position tracker should match half a step") {
                    REQUIRE(handler.get_current_position() == default_velocity);
                }
            }
        }
    }

    GIVEN("a duration of 2 ticks and velocity at half a step per tick") {
        MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                              mock_message_queue::MockMessageQueue>
            handler{};
        WHEN("moving at half a step per tick") {
            Move msg1 = Move{.duration = 2, .velocity = default_velocity};
            handler.set_buffered_move(msg1);
            THEN("motor would step in the 2nd tick") {
                REQUIRE(!handler.tick());
                REQUIRE(handler.tick());
                AND_THEN("the position tracker should match exactly one step") {
                    auto distance_traveled =
                        static_cast<uint64_t>(default_velocity) * 2;
                    REQUIRE(handler.get_current_position() ==
                            distance_traveled);
                }
            }
        }
    }

    GIVEN("a motor handler") {
        MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                              mock_message_queue::MockMessageQueue>
            handler{};
        auto velocity =
            static_cast<sq0_31>(0.3 * static_cast<double>(1LL << (TO_RADIX)));
        auto msg1 = Move{.duration = 4, .velocity = velocity};
        handler.set_buffered_move(msg1);

        WHEN("you move at +0.3 velocity") {
            REQUIRE(!handler.tick());
            REQUIRE(!handler.tick());
            REQUIRE(!handler.tick());

            THEN("the fourth tick should step") {
                REQUIRE(handler.tick());
                REQUIRE(handler.get_current_position() ==
                        static_cast<q31_31>(velocity) * 4);

                AND_WHEN("moving in the other direction") {
                    msg1.velocity = -velocity;
                    handler.set_buffered_move(msg1);

                    THEN("the first tick should step") {
                        REQUIRE(handler.tick());
                        REQUIRE(!handler.tick());
                        REQUIRE(!handler.tick());
                        REQUIRE(!handler.tick());
                        REQUIRE(handler.get_current_position() == 0x0);
                    }
                }
            }
        }
    }
}

TEST_CASE("Non-zero acceleration") {
    GIVEN("a motor handler") {
        MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                              mock_message_queue::MockMessageQueue>
            handler{};
        WHEN("move starts at 0 velocity with positive acceleration") {
            auto msg = Move{.duration = 3,
                            .velocity = 0,
                            .acceleration = 1 << (TO_RADIX - 2)};
            handler.set_buffered_move(msg);
            THEN("the third tick should step") {
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(handler.tick());
                REQUIRE(handler.get_current_position() ==
                        ((1LL << (TO_RADIX)) + (1LL << (TO_RADIX - 1))));
            }
        }
        WHEN("move starts at 0 velocity with negative acceleration") {
            handler.set_current_position((1LL << (TO_RADIX)) +
                                         (1LL << (TO_RADIX - 1)));
            auto msg = Move{.duration = 3,
                            .velocity = 0,
                            .acceleration = -(1 << (TO_RADIX - 2))};
            handler.set_buffered_move(msg);
            THEN("only the second tick should step") {
                REQUIRE(!handler.tick());
                REQUIRE(handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(handler.get_current_position() == 0x0);
            }
        }

        WHEN("move starts at positive velocity with negative acceleration") {
            auto msg = Move{.duration = 3,
                            .velocity = 1 << (TO_RADIX - 1),
                            .acceleration = -(1 << (TO_RADIX - 3))};
            handler.set_buffered_move(msg);
            handler.set_current_position(0x70000000);
            THEN("the third and fifth ticks should step") {
                REQUIRE(handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(handler.get_current_position() ==
                        (static_cast<q31_31>(0x70000000) + 0x60000000));
            }
        }
        WHEN("move starts at negative velocity with positive acceleration") {
            auto msg = Move{.duration = 4,
                            .velocity = -1 << (TO_RADIX - 1),
                            .acceleration = 1 << (TO_RADIX - 2)};
            handler.set_buffered_move(msg);
            THEN("the third and forth ticks should step") {
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(!handler.tick());
                REQUIRE(handler.tick());
                REQUIRE(handler.get_current_position() ==
                        (-1LL << (TO_RADIX)) + (5LL << (TO_RADIX - 1)));
            }
        }
    }
}

TEST_CASE("Compute move sequence") {
    GIVEN("a motor handler") {
        MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                              mock_message_queue::MockMessageQueue>
            handler{};
        mock_message_queue::MockMessageQueue<Move> queue;
        mock_message_queue::MockMessageQueue<Ack> completed_queue;
        handler.set_message_queue(&queue, &completed_queue);

        handler.set_current_position(0x0);
        auto msg1 = Move{.duration = 2, .velocity = default_velocity};
        queue.try_write_isr(msg1);
        handler.update_move();

        WHEN("a move duration is up, and the queue is empty") {
            for (int i = 0; i < 2; i++) {
                handler.pulse();
            }

            THEN("we do not move") {
                handler.pulse();
                REQUIRE(!handler.can_step());
                REQUIRE(!handler.has_active_move);
            }
        }
        WHEN("a move duration is up, and there is a move in the queue") {
            auto msg2 = Move{.duration = 5, .velocity = -default_velocity};
            queue.try_write_isr(msg2);

            for (int i = 0; i < 2; i++) {
                handler.pulse();
            }

            THEN("we immediately switch to the new move") {
                handler.pulse();
                REQUIRE(handler.can_step());
                REQUIRE(handler.has_active_move);
                REQUIRE(handler.get_buffered_move().velocity == msg2.velocity);
                REQUIRE(handler.get_buffered_move().duration == msg2.duration);
            }
        }
    }
}

TEST_CASE("moves that result in out of range positions") {
    MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                          mock_message_queue::MockMessageQueue>
        handler{};
    mock_message_queue::MockMessageQueue<Move> queue;
    mock_message_queue::MockMessageQueue<Ack> completed_queue;
    handler.set_message_queue(&queue, &completed_queue);

    GIVEN("Move past the largest possible value for position") {
        // Reaching the max possible position will probably be almost
        // impossible to do in the system. Rather than testing that tick
        // detects this type of overflow, we should just check that the
        // overflow function works correctly for very large numbers.

        q31_31 position_1 = 0x4000000000000000;
        q31_31 position_2 = position_1 + position_1;
        REQUIRE(handler.overflow(position_1, position_2));
    }

    GIVEN("Move past the lowest possible value for position (zero)") {
        // Here we should give a negative number
        q31_31 current_position = 0x0;
        const sq0_31 neg_velocity = -default_velocity;
        handler.set_current_position(current_position);
        Move msg1 = Move{.duration = 2, .velocity = neg_velocity};
        handler.set_buffered_move(msg1);

        THEN(
            "the motor should not pulse, and position should be "
            "unchanged.") {
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
