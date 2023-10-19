#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"
#include "motor-control/tests/mock_motor_driver_client.hpp"

using namespace motor_handler;

#define TO_RADIX 31

static constexpr sq0_31 default_velocity =
    0x1 << (TO_RADIX - 1);  // half a step per tick

static constexpr float tick_per_um = 1;
static constexpr uint32_t stall_threshold_um = 10;

struct HandlerContainer {
    test_mocks::MockMotorHardware hw{};
    test_mocks::MockMessageQueue<Move> queue{};
    test_mocks::MockMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>
        update_position_queue{};
    test_mocks::MockMoveStatusReporterClient reporter{};
    test_mocks::MockMotorDriverClient driver{};
    stall_check::StallCheck stall{tick_per_um, tick_per_um, stall_threshold_um};
    MotorInterruptHandler<test_mocks::MockMessageQueue,
                          test_mocks::MockMoveStatusReporterClient, 
                          test_mocks::MockMotorDriverClient, Move,
                          test_mocks::MockMotorHardware>
        handler{queue, reporter, driver, hw, stall, update_position_queue};
};

sq0_31 convert_velocity(float f) {
    return sq0_31(f * static_cast<float>(1LL << TO_RADIX));
}

q31_31 convert_distance(float f) {
    return q31_31(f * static_cast<float>(1LL << TO_RADIX));
}

q31_31 get_distance(sq0_31 velocity, sq0_31 acceleration, uint64_t duration) {
    int64_t s_duration = duration;
    int64_t dist_1 = velocity * s_duration;
    int64_t dist_2 = acceleration * s_duration * (s_duration + 1LL) / 2LL;
    return q31_31(dist_1 + dist_2);
}

TEST_CASE("Constant velocity") {
    GIVEN("a duration of 1 tick and velocity at half a step per tick") {
        HandlerContainer test_objs{};
        test_objs.handler.set_current_position(0x0);
        WHEN("moving at half a step per tick") {
            Move msg1 = Move{.duration = 1, .velocity = convert_velocity(0.5)};
            test_objs.handler.set_buffered_move(msg1);
            THEN("motor would not step in the 1st tick") {
                REQUIRE(!test_objs.handler.tick());
                AND_THEN("the position tracker should match half a step") {
                    REQUIRE(test_objs.handler.get_current_position() ==
                            q31_31(convert_velocity(0.5)));
                }
            }
        }
    }

    GIVEN("a duration of 2 ticks and velocity at half a step per tick") {
        HandlerContainer test_objs{};
        WHEN("moving at half a step per tick") {
            Move msg1 = Move{.duration = 2, .velocity = convert_velocity(0.5)};
            test_objs.handler.set_buffered_move(msg1);
            THEN("motor would step in the 2nd tick") {
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.tick());
                AND_THEN("the position tracker should match exactly one step") {
                    auto distance_traveled = q31_31(1LL << TO_RADIX);
                    REQUIRE(test_objs.handler.get_current_position() ==
                            distance_traveled);
                }
            }
        }
    }

    GIVEN("a motor handler") {
        HandlerContainer test_objs{};
        auto velocity = convert_velocity(0.3);
        auto msg1 = Move{.duration = 4, .velocity = velocity};
        test_objs.handler.set_buffered_move(msg1);

        WHEN("you move at +0.3 velocity") {
            REQUIRE(!test_objs.handler.tick());
            REQUIRE(!test_objs.handler.tick());
            REQUIRE(!test_objs.handler.tick());

            THEN("the fourth tick should step") {
                auto initial_distance =
                    get_distance(velocity, 0, msg1.duration);
                REQUIRE(test_objs.handler.tick());
                REQUIRE(test_objs.handler.get_current_position() ==
                        initial_distance);

                AND_WHEN("moving in the other direction") {
                    msg1.duration = 2;
                    msg1.velocity = convert_velocity(-0.3);
                    test_objs.handler.set_buffered_move(msg1);

                    THEN("the first tick should step") {
                        REQUIRE(test_objs.handler.tick());
                        REQUIRE(!test_objs.handler.tick());
                        REQUIRE(test_objs.handler.get_current_position() ==
                                initial_distance + get_distance(msg1.velocity,
                                                                0,
                                                                msg1.duration));
                    }
                }
            }
        }
    }
}

TEST_CASE("Non-zero acceleration") {
    GIVEN("a motor handler") {
        HandlerContainer test_objs{};
        WHEN("move starts at 0 velocity with positive acceleration") {
            auto acceleration = convert_velocity(0.25);
            auto msg = Move{
                .duration = 3, .velocity = 0, .acceleration = acceleration};

            test_objs.handler.set_buffered_move(msg);
            THEN("the third tick should step") {
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.tick());
                REQUIRE(test_objs.handler.get_current_position() ==
                        convert_distance(0.25 + 0.5 + 0.75));
            }
        }
        WHEN("move starts at 0 velocity with negative acceleration") {
            test_objs.handler.set_current_position(
                convert_distance(0.25 + 0.5 + 0.75));
            auto acceleration = convert_velocity(-0.25);
            auto msg = Move{
                .duration = 3, .velocity = 0, .acceleration = acceleration};
            test_objs.handler.set_buffered_move(msg);
            THEN("only the second tick should step") {
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.tick());
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.get_current_position() == 0x0);
            }
        }

        WHEN("move starts at positive velocity with negative acceleration") {
            sq0_31 velocity = convert_velocity(0.5);
            sq0_31 acceleration = convert_velocity(-0.025);
            q31_31 duration = 3;
            auto msg = Move{.duration = duration,
                            .velocity = velocity,
                            .acceleration = acceleration};
            test_objs.handler.set_buffered_move(msg);
            THEN("the third and fifth ticks should step") {
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.tick());
                REQUIRE(test_objs.handler.get_current_position() ==
                        get_distance(velocity, acceleration, duration));
            }
        }
        WHEN("move starts at negative velocity with positive acceleration") {
            sq0_31 velocity = convert_velocity(-0.5);
            sq0_31 acceleration = convert_velocity(0.05);
            q31_31 duration = 4;
            test_objs.handler.set_current_position(convert_distance(10.0));
            auto msg = Move{.duration = duration,
                            .velocity = velocity,
                            .acceleration = acceleration};
            test_objs.handler.set_buffered_move(msg);
            THEN("the third and forth ticks should step") {
                REQUIRE(test_objs.handler.tick());
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.tick());
                REQUIRE(!test_objs.handler.tick());
                REQUIRE(test_objs.handler.get_current_position() ==
                        convert_distance(10.0) +
                            get_distance(velocity, acceleration, duration));
            }
        }
    }
}

TEST_CASE("Compute move sequence") {
    GIVEN("a motor handler") {
        HandlerContainer test_objs{};

        test_objs.handler.set_current_position(0x0);
        auto msg1 = Move{.duration = 2, .velocity = default_velocity};
        test_objs.queue.try_write_isr(msg1);
        test_objs.handler.update_move();

        WHEN("a move duration is up, and the queue is empty") {
            for (int i = 0; i < 2; i++) {
                static_cast<void>(test_objs.handler.pulse());
            }

            THEN("we do not move") {
                static_cast<void>(test_objs.handler.pulse());
                REQUIRE(!test_objs.handler.can_step());
                REQUIRE(!test_objs.handler.has_active_move());
            }
        }
        WHEN("a move duration is up, and there is a move in the queue") {
            auto msg2 = Move{.duration = 5, .velocity = -default_velocity};
            test_objs.queue.try_write_isr(msg2);

            for (int i = 0; i < 2; i++) {
                static_cast<void>(test_objs.handler.pulse());
            }

            THEN("we immediately switch to the new move") {
                static_cast<void>(test_objs.handler.pulse());
                REQUIRE(test_objs.handler.can_step());
                REQUIRE(test_objs.handler.has_active_move());
                REQUIRE(test_objs.handler.get_buffered_move().velocity ==
                        msg2.velocity);
                REQUIRE(test_objs.handler.get_buffered_move().duration ==
                        msg2.duration);
            }
        }
    }
}

TEST_CASE("moves that result in out of range positions") {
    HandlerContainer test_objs{};

    GIVEN("Move past the largest possible value for position") {
        // Reaching the max possible position will probably be almost
        // impossible to do in the system. Rather than testing that tick
        // detects this type of overflow, we should just check that the
        // overflow function works correctly for very large numbers.

        q31_31 position_1 = 0x4000000000000000;
        q31_31 position_2 = position_1 + position_1;
        REQUIRE(test_objs.handler.overflow(position_1, position_2));
    }

    GIVEN("Move past the lowest possible value for position (zero)") {
        // Here we should give a negative number
        q31_31 current_position = 0x0;
        const sq0_31 neg_velocity = -default_velocity;
        test_objs.handler.set_current_position(current_position);
        Move msg1 = Move{.duration = 2, .velocity = neg_velocity};
        test_objs.handler.set_buffered_move(msg1);

        THEN(
            "the motor should not pulse, and position should be "
            "unchanged.") {
            // (TODO lc): we don't have true error handling yet
            // this should probably return some type of error in
            // the instance of an overflow.
            REQUIRE(!test_objs.handler.tick());
            REQUIRE(test_objs.handler.get_current_position() ==
                    current_position);
            AND_THEN("the move should be finished") {
                static_cast<void>(test_objs.handler.pulse());
                REQUIRE(test_objs.handler.has_active_move() == false);
            }
        }
    }
}

TEST_CASE("Changing motor direction") {
    HandlerContainer test_objs{};

    test_objs.handler.set_current_position(0x0);

    GIVEN("Positive move velocity") {
        auto msg1 = Move{.duration = 2, .velocity = default_velocity};
        test_objs.queue.try_write_isr(msg1);
        test_objs.handler.update_move();

        for (int i = 0; i < 2; i++) {
            static_cast<void>(test_objs.handler.tick());
            REQUIRE(test_objs.handler.set_direction_pin());
        }

        THEN("Negative move velocity") {
            auto msg1 = Move{.duration = 2, .velocity = -default_velocity};
            test_objs.queue.try_write_isr(msg1);
            test_objs.handler.update_move();
            for (int i = 0; i < 2; i++) {
                static_cast<void>(test_objs.handler.tick());
                REQUIRE(!test_objs.handler.set_direction_pin());
            }
        }
    }
}

TEST_CASE("Finishing a move") {
    HandlerContainer test_objs{};

    GIVEN("a non-homing move when not homed") {
        auto move = Move{.group_id = 1, .seq_id = 2};
        test_objs.handler.set_buffered_move(move);
        uint64_t set_position = static_cast<uint64_t>(100) << 31;
        uint32_t set_encoder_position = static_cast<uint32_t>(200);
        test_objs.handler.set_current_position(set_position);
        test_objs.hw.sim_set_encoder_pulses(set_encoder_position);
        test_objs.handler.finish_current_move();

        THEN(
            "the ack message should contain the correct information when the "
            "move finishes") {
            REQUIRE(test_objs.reporter.messages.size() == 1);
            auto msg = std::get<Ack>(test_objs.reporter.messages[0]);
            REQUIRE(msg.group_id == move.group_id);
            REQUIRE(msg.seq_id == move.seq_id);
            REQUIRE(msg.current_position_steps == 100);
            REQUIRE(msg.encoder_position == 200);
            REQUIRE(msg.position_flags == 0);
        }
    }

    GIVEN("a homing move") {
        auto move = Move{.group_id = 1,
                         .seq_id = 2,
                         .stop_condition = static_cast<uint8_t>(
                             MoveStopCondition::limit_switch)};
        test_objs.handler.set_buffered_move(move);
        uint64_t set_position = static_cast<uint64_t>(100) << 31;
        uint32_t set_encoder_position = static_cast<uint32_t>(200);
        test_objs.handler.set_current_position(set_position);
        test_objs.hw.sim_set_encoder_pulses(set_encoder_position);
        test_objs.hw.set_mock_lim_sw(true);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::encoder_position_ok);
        REQUIRE(test_objs.handler.homing_stopped());

        THEN(
            "the ack message should contain the correct information when the "
            "move finishes") {
            REQUIRE(test_objs.reporter.messages.size() == 1);
            auto msg = std::get<Ack>(test_objs.reporter.messages[0]);
            REQUIRE(msg.group_id == move.group_id);
            REQUIRE(msg.seq_id == move.seq_id);
            REQUIRE(msg.current_position_steps == 0);
            REQUIRE(msg.encoder_position == 200);
            REQUIRE(msg.position_flags == 0x0);

            AND_GIVEN("a backoff move") {
                test_objs.reporter.messages.clear();
                move = Move{.group_id = 1,
                            .seq_id = 3,
                            .stop_condition = static_cast<uint8_t>(
                                MoveStopCondition::limit_switch_backoff),
                            .start_encoder_position =
                                test_objs.hw.get_encoder_pulses()};
                test_objs.handler.set_buffered_move(move);
                uint64_t set_position = static_cast<uint64_t>(10) << 31;
                uint32_t set_encoder_position = static_cast<uint32_t>(220);
                test_objs.handler.set_current_position(set_position);
                test_objs.hw.sim_set_encoder_pulses(set_encoder_position);
                test_objs.hw.set_mock_lim_sw(false);
                REQUIRE(test_objs.handler.backed_off());

                THEN(
                    "the ack message should contain the correct information "
                    "when the move finishes") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    auto msg = std::get<Ack>(test_objs.reporter.messages[0]);
                    REQUIRE(msg.group_id == move.group_id);
                    REQUIRE(msg.seq_id == move.seq_id);
                    REQUIRE(msg.current_position_steps == 0);
                    REQUIRE(msg.encoder_position == 0);
                    REQUIRE(msg.position_flags == 0x3);
                    REQUIRE(msg.start_encoder_position == -20);
                }

                AND_GIVEN("a subsequent move") {
                    test_objs.reporter.messages.clear();
                    move = Move{.group_id = 1, .seq_id = 2};
                    test_objs.handler.set_buffered_move(move);
                    uint64_t set_position = static_cast<uint64_t>(100) << 31;
                    uint32_t set_encoder_position = static_cast<uint32_t>(200);
                    test_objs.handler.set_current_position(set_position);
                    test_objs.hw.sim_set_encoder_pulses(set_encoder_position);
                    test_objs.handler.finish_current_move();

                    THEN(
                        "the ack message should contain the correct "
                        "information "
                        "when the move finishes") {
                        REQUIRE(test_objs.reporter.messages.size() == 1);
                        auto msg =
                            std::get<Ack>(test_objs.reporter.messages[0]);
                        REQUIRE(msg.group_id == move.group_id);
                        REQUIRE(msg.seq_id == move.seq_id);
                        REQUIRE(msg.current_position_steps == 100);
                        REQUIRE(msg.encoder_position == 200);
                        REQUIRE(msg.position_flags == 0x3);
                    }
                }
            }
        }
    }
}
