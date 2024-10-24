#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace motor_handler;

#define TO_RADIX 31

struct HandlerContainer {
    test_mocks::MockMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::Move> queue{};
    test_mocks::MockMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>
        update_position_queue{};
    test_mocks::MockMoveStatusReporterClient reporter{};
    stall_check::StallCheck stall{10, 10, 10};
    MotorInterruptHandler<test_mocks::MockMessageQueue,
                          test_mocks::MockMoveStatusReporterClient, Move,
                          test_mocks::MockMotorHardware>
        handler{queue, reporter, hw, stall, update_position_queue};
};

static constexpr sq0_31 default_velocity =
    0x1 << (TO_RADIX - 1);  // half a step per tick

sq0_31 convert_velocity_to(float f) {
    return sq0_31(f * static_cast<float>(1LL << TO_RADIX));
}

SCENARIO("MoveStopCondition::limit_switch with the limit switch triggered") {
    HandlerContainer test_objs{};
    Move msg1 = Move{.duration = 2,
                     .velocity = convert_velocity_to(-.5),
                     .acceleration = 0,
                     .group_id = 1,
                     .seq_id = 0,
                     .stop_condition =
                         static_cast<uint8_t>(MoveStopCondition::limit_switch)};

    test_objs.queue.try_write_isr(msg1);
    test_objs.handler.set_current_position(0x500);
    test_objs.hw.set_mock_lim_sw(false);
    test_objs.hw.position_flags.set_flag(
        MotorPositionStatus::Flags::stepper_position_ok);

    WHEN("the move is loaded") {
        test_objs.handler.update_move();
        REQUIRE(test_objs.handler.has_active_move());

        THEN("position gets set to large positive number") {
            REQUIRE(test_objs.handler.get_current_position() ==
                    0x7FFFFFFFFFFFFFFF);
        }

        THEN("stepper and encoder position flags are cleared") {
            REQUIRE(!test_objs.hw.position_flags.check_flag(
                MotorPositionStatus::Flags::stepper_position_ok));
            REQUIRE(!test_objs.hw.position_flags.check_flag(
                MotorPositionStatus::Flags::encoder_position_ok));
        }

        AND_WHEN("the limit switch has been triggered") {
            for (int i = 0; i < (int)msg1.duration; ++i) {
                if (i == (int)msg1.duration - 1) {
                    test_objs.handler.set_current_position(
                        static_cast<uint64_t>(350) << 31);
                    test_objs.hw.sim_set_encoder_pulses(50);
                    test_objs.hw.set_mock_lim_sw(true);
                }
                test_objs.handler.run_interrupt();
            }

            THEN(
                "the move should be stopped with ack id = stopped "
                "by "
                "condition") {
                REQUIRE(test_objs.reporter.messages.size() >= 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
                REQUIRE(read_ack.encoder_position == 50);
                REQUIRE(read_ack.current_position_steps == 0);
            }
            THEN(
                "the stepper position flag and encoder position flags are "
                "still cleared") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    MotorPositionStatus::Flags::stepper_position_ok));
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    MotorPositionStatus::Flags::encoder_position_ok));
            }
        }
    }
}

SCENARIO("MoveStopCondition::limit_switch and limit switch is not triggered") {
    HandlerContainer test_objs{};
    Move msg1 = Move{.duration = 2,
                     .velocity = convert_velocity_to(-.5),
                     .acceleration = 0,
                     .group_id = 1,
                     .seq_id = 0,
                     .stop_condition =
                         static_cast<uint8_t>(MoveStopCondition::limit_switch)};

    test_objs.queue.try_write_isr(msg1);
    test_objs.handler.set_current_position(0xFFFFFFFFFF);
    test_objs.hw.set_mock_lim_sw(false);
    test_objs.hw.position_flags.set_flag(
        MotorPositionStatus::Flags::stepper_position_ok);

    WHEN("the move is loaded") {
        test_objs.handler.update_move();
        REQUIRE(test_objs.handler.has_active_move());

        THEN("position gets set to large positive number") {
            REQUIRE(test_objs.handler.get_current_position() ==
                    0x7FFFFFFFFFFFFFFF);
        }
        THEN("stepper and encoder position flags are cleared") {
            REQUIRE(!test_objs.hw.position_flags.check_flag(
                MotorPositionStatus::Flags::stepper_position_ok));
            REQUIRE(!test_objs.hw.position_flags.check_flag(
                MotorPositionStatus::Flags::encoder_position_ok));
        }

        AND_WHEN("the limit switch has not been triggered") {
            for (int i = 0; i < (int)msg1.duration + 1; ++i) {
                if (i == (int)msg1.duration) {
                    test_objs.handler.set_current_position(
                        static_cast<uint64_t>(350) << 31);
                    test_objs.hw.sim_set_encoder_pulses(50);
                }
                test_objs.handler.run_interrupt();
            }

            THEN(
                "the move should be stopped with ack id = stopped "
                "without "
                "condition") {
                REQUIRE(test_objs.reporter.messages.size() == 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.front());
                REQUIRE(read_ack.encoder_position == 50);
                REQUIRE(read_ack.current_position_steps == 350);
                REQUIRE(read_ack.ack_id ==
                        AckMessageId::complete_without_condition);
            }
            THEN("position should not be reset") {
                REQUIRE(!test_objs.handler.get_current_position() == 0);
            }
            THEN("stepper and encoder position flags should remained cleared") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    MotorPositionStatus::Flags::stepper_position_ok));
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    MotorPositionStatus::Flags::encoder_position_ok));
            }
        }
    }
}

SCENARIO("Not MoveStopCondition::limit_switch but limit switch triggered") {
    HandlerContainer test_objs{};
    test_objs.handler.set_current_position(0x0);
    Move msg1 =
        Move{.duration = 2,
             .velocity = convert_velocity_to(.5),
             .acceleration = 0,
             .group_id = 1,
             .seq_id = 0,
             .stop_condition = static_cast<uint8_t>(MoveStopCondition::none)};
    GIVEN("Move with stop condition = none in progress") {
        test_objs.queue.try_write_isr(msg1);
        test_objs.hw.set_mock_lim_sw(true);
        WHEN("the first move is loaded") {
            CHECK(!test_objs.handler.pulse());
            THEN("position is not immediately changed") {
                REQUIRE(test_objs.handler.get_current_position() == 0);
            }
            AND_WHEN("the move is finished") {
                REQUIRE(!test_objs.handler.pulse());
                REQUIRE(test_objs.handler.pulse());
                REQUIRE(!test_objs.handler.pulse());
                THEN(
                    "the move should have ack_id complete_without_condition, "
                    "and position = 4") {
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::complete_without_condition);
                }
            }
        }
    }
}
