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

SCENARIO(
    "MoveStopCondition::limit_switch_backoff with the limit switch released") {
    HandlerContainer test_objs{};
    Move msg1 =
        Move{.duration = 2,
             .velocity = int32_t(0.5 * static_cast<float>(1LL << TO_RADIX)),
             .acceleration = 0,
             .group_id = 1,
             .seq_id = 0,
             .stop_condition =
                 static_cast<uint8_t>(MoveStopCondition::limit_switch_backoff)};

    test_objs.queue.try_write_isr(msg1);
    test_objs.handler.set_current_position(0x0);
    test_objs.hw.set_mock_lim_sw(true);

    GIVEN("stepper and encoder position are not okay before the move") {
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::encoder_position_ok);

        WHEN("the move is loaded") {
            test_objs.handler.update_move();
            REQUIRE(test_objs.handler.has_active_move());

            THEN("the limit switch has been released") {
                for (int i = 0; i < (int)msg1.duration; ++i) {
                    if (i == (int)msg1.duration - 1) {
                        test_objs.handler.set_current_position(
                            static_cast<uint64_t>(350) << 31);
                        test_objs.hw.sim_set_encoder_pulses(50);
                        test_objs.hw.set_mock_lim_sw(false);
                    }
                    test_objs.handler.run_interrupt();
                }
                AND_THEN(
                    "the move should be stopped with ack id = stopped by "
                    "condition") {
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::stopped_by_condition);
                    REQUIRE(read_ack.encoder_position == 0);
                    REQUIRE(read_ack.current_position_steps == 0);
                    REQUIRE(read_ack.position_flags == 0x3);
                }

                THEN("position should be reset") {
                    REQUIRE(test_objs.handler.get_current_position() == 0);
                }

                THEN("both stepper and encoder position should be ok") {
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::encoder_position_ok));
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::stepper_position_ok));
                }
            }
        }
    }

    GIVEN("stepper position is okay before the move") {
        test_objs.hw.position_flags.set_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        test_objs.hw.position_flags.set_flag(
            MotorPositionStatus::Flags::encoder_position_ok);

        WHEN("the move is loaded") {
            test_objs.handler.update_move();
            REQUIRE(test_objs.handler.has_active_move());

            AND_WHEN("the limit switch has been released") {
                for (int i = 0; i < (int)msg1.duration; ++i) {
                    if (i == (int)msg1.duration - 1) {
                        test_objs.handler.set_current_position(
                            static_cast<uint64_t>(350) << 31);
                        test_objs.hw.sim_set_encoder_pulses(50);
                        test_objs.hw.set_mock_lim_sw(false);
                    }
                    test_objs.handler.run_interrupt();
                }

                AND_THEN(
                    "the move should be stopped with ack id = stopped by "
                    "condition") {
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::stopped_by_condition);
                    REQUIRE(read_ack.encoder_position == 0);
                    REQUIRE(read_ack.current_position_steps == 0);
                    REQUIRE(read_ack.position_flags == 0x3);
                }

                THEN("position should be reset") {
                    REQUIRE(test_objs.handler.get_current_position() == 0);
                }

                THEN("both stepper and encoder position should be ok") {
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::encoder_position_ok));
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::stepper_position_ok));
                }
            }
        }
    }
}

SCENARIO(
    "MoveStopCondition::limit_switch_backoff and limit switch is not "
    "released") {
    HandlerContainer test_objs{};
    Move msg1 =
        Move{.duration = 2,
             .velocity = int32_t(0.5 * static_cast<float>(1LL << TO_RADIX)),
             .acceleration = 0,
             .group_id = 1,
             .seq_id = 0,
             .stop_condition =
                 static_cast<uint8_t>(MoveStopCondition::limit_switch_backoff)};

    test_objs.queue.try_write_isr(msg1);
    test_objs.handler.set_current_position(0x0);
    test_objs.hw.set_mock_lim_sw(true);

    GIVEN("stepper and encoder positions are not okay") {
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::encoder_position_ok);
        WHEN("the move is loaded") {
            test_objs.handler.update_move();
            REQUIRE(test_objs.handler.has_active_move());

            AND_WHEN("the limit switch has not been triggered") {
                for (int i = 0; i < (int)msg1.duration; ++i) {
                    test_objs.handler.run_interrupt();
                }

                test_objs.handler.set_current_position(
                    static_cast<uint64_t>(350) << 31);
                test_objs.hw.sim_set_encoder_pulses(50);
                test_objs.handler.run_interrupt();
                THEN(
                    "the move should be stopped with ack id = stopped without "
                    "condition") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(read_ack.encoder_position == 50);
                    REQUIRE(read_ack.current_position_steps == 350);
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::complete_without_condition);
                    REQUIRE(read_ack.position_flags == 0x0);
                }
                THEN("position should not be reset") {
                    REQUIRE(!test_objs.handler.get_current_position() == 0);
                }

                THEN(
                    "both stepper and encoder position should still be not "
                    "ok") {
                    REQUIRE(!test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::encoder_position_ok));
                    REQUIRE(!test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::stepper_position_ok));
                }
            }
        }
    }

    GIVEN("stepper and encoder position statuses are okay") {
        test_objs.hw.position_flags.set_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        test_objs.hw.position_flags.set_flag(
            MotorPositionStatus::Flags::encoder_position_ok);

        WHEN("the move is loaded") {
            test_objs.handler.update_move();
            REQUIRE(test_objs.handler.has_active_move());

            AND_WHEN("the limit switch has not been triggered") {
                for (int i = 0; i < (int)msg1.duration; ++i) {
                    test_objs.handler.run_interrupt();
                }

                test_objs.handler.set_current_position(
                    static_cast<uint64_t>(350) << 31);
                test_objs.hw.sim_set_encoder_pulses(50);
                test_objs.handler.run_interrupt();

                THEN(
                    "the move should be stopped with ack id = stopped without "
                    "condition") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::complete_without_condition);
                    REQUIRE(read_ack.encoder_position == 50);
                    REQUIRE(read_ack.current_position_steps == 350);
                    REQUIRE(read_ack.position_flags == 0x3);
                }
                THEN("position should not be reset") {
                    REQUIRE(!test_objs.handler.get_current_position() == 0);
                }

                THEN(
                    "both stepper and encoder position should not have "
                    "changed") {
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::encoder_position_ok));
                    REQUIRE(test_objs.hw.position_flags.check_flag(
                        MotorPositionStatus::Flags::stepper_position_ok));
                }
                AND_WHEN("Motion happens after the move.") {
                    test_objs.hw.sim_set_encoder_pulses(200);
                    test_objs.reporter.messages.clear();
                    for (int i = 0; i < 10; ++i) {
                        test_objs.handler.run_interrupt();
                    }
                    THEN("No error is sent.") {
                        REQUIRE(test_objs.reporter.messages.size() == 0);
                    }
                }
            }
        }
    }
}
