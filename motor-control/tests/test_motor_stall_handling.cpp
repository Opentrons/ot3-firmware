#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/usage_messages.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"
#include "motor-control/tests/mock_motor_driver_client.hpp"

using namespace motor_handler;

#define TO_RADIX 31

static constexpr float tick_per_um = 1;
static constexpr uint32_t stall_threshold_um = 5;
static constexpr sq0_31 default_velocity =
    sq0_31(0.5 * static_cast<float>(1LL << TO_RADIX));

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

SCENARIO("motor handler stall detection") {
    using Flags = MotorPositionStatus::Flags;
    using Stops = MoveStopCondition;

    HandlerContainer test_objs{};
    test_objs.handler.set_current_position(0x0);
    test_objs.hw.sim_set_encoder_pulses(0);
    test_objs.hw.position_flags.set_flag(
        MotorPositionStatus::Flags::encoder_position_ok);
    test_objs.hw.position_flags.set_flag(
        MotorPositionStatus::Flags::stepper_position_ok);

    GIVEN("a linear move which is not expecting a stall") {
        auto cond = GENERATE(Stops::none, Stops::sync_line);
        auto msg1 = Move{.message_index = 101,
                         .duration = 23,
                         .velocity = default_velocity,
                         .stop_condition = static_cast<uint8_t>(cond)};
        auto msg2 = Move{.duration = 10, .velocity = default_velocity};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 2);
        test_objs.handler.update_move();

        WHEN("encoder doesn't update with the motor") {
            REQUIRE(test_objs.queue.get_size() == 1);
            REQUIRE(test_objs.reporter.messages.size() == 0);
            for (int i = 0; i < (int)msg1.duration; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN("a unrecoverable collision error is raised") {
                    can::messages::ErrorMessage err =
                        std::get<can::messages::ErrorMessage>(
                            test_objs.reporter.messages[0]);
                    REQUIRE(err.message_index == 101);
                    REQUIRE(err.error_code ==
                            can::ids::ErrorCode::collision_detected);
                    REQUIRE(err.severity ==
                            can::ids::ErrorSeverity::unrecoverable);
                    THEN("a move complete with position error ack is sent") {
                        Ack ack_msg =
                            std::get<Ack>(test_objs.reporter.messages[1]);
                        REQUIRE(ack_msg.ack_id == AckMessageId::position_error);
                        REQUIRE(ack_msg.message_index == 101);
                        usage_messages::IncreaseErrorCount inc_error_count =
                            std::get<usage_messages::IncreaseErrorCount>(
                                test_objs.reporter.messages[2]);
                        std::ignore = inc_error_count;
                    }
                }
            }
        }
    }

    GIVEN("a move with ignore_stalls stop condition") {
        auto cond =
            static_cast<uint8_t>(GENERATE(Stops::none, Stops::sync_line));
        auto msg1 =
            Move{.message_index = 13,
                 .duration = 26,
                 .velocity = default_velocity,
                 .stop_condition = static_cast<uint8_t>(
                     static_cast<uint8_t>(Stops::ignore_stalls) ^ cond)};
        auto msg2 =
            Move{.duration = 5,
                 .velocity = default_velocity,
                 .stop_condition = static_cast<uint8_t>(
                     static_cast<uint8_t>(Stops::ignore_stalls) ^ cond)};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 2);
        test_objs.handler.update_move();
        WHEN("encoder doesn't update with the motor") {
            for (int i = 0; i < 23; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN(
                "the stall is detected but it is ignored, and a warning is "
                "sent") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.message_index == 13);
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);
                REQUIRE(err.severity == can::ids::ErrorSeverity::warning);
                usage_messages::IncreaseErrorCount inc_error_count =
                    std::get<usage_messages::IncreaseErrorCount>(
                        test_objs.reporter.messages.back());
                std::ignore = inc_error_count;

                THEN("the move finishes") {
                    for (int i = 22; i < (int)msg1.duration; i++) {
                        test_objs.handler.run_interrupt();
                    }
                    REQUIRE(test_objs.reporter.messages.size() == 3);
                    Ack ack_msg =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(ack_msg.ack_id ==
                            AckMessageId::complete_without_condition);
                    REQUIRE(ack_msg.message_index == 13);
                    test_objs.reporter.messages.clear();
                    WHEN("the interrupt runs again") {
                        for (int i = 0; i < (int)msg2.duration; ++i) {
                            test_objs.handler.run_interrupt();
                        }
                        THEN("the second move will be executed as normal") {
                            REQUIRE(!test_objs.hw.position_flags.check_flag(
                                Flags::stepper_position_ok));
                            REQUIRE(test_objs.reporter.messages.size() == 1);
                            Ack ack_msg = std::get<Ack>(
                                test_objs.reporter.messages.back());
                            REQUIRE(ack_msg.ack_id ==
                                    AckMessageId::complete_without_condition);
                        }
                    }
                }
            }
        }
    }

    GIVEN("a move with ignore_stalls stop condition with long duration") {
        auto cond =
            static_cast<uint8_t>(GENERATE(Stops::none, Stops::sync_line));
        auto msg1 =
            Move{.message_index = 13,
                 .duration = 100,
                 .velocity = default_velocity,
                 .stop_condition = static_cast<uint8_t>(
                     static_cast<uint8_t>(Stops::ignore_stalls) ^ cond)};
        test_objs.queue.try_write(msg1);
        test_objs.handler.update_move();
        WHEN("multiple stalls detected during the move") {
            int stall_counter = 0;
            for (int i = 0; i < (int)msg1.duration + 1; ++i) {
                test_objs.handler.run_interrupt();
                if (test_objs.handler.stall_detected()) {
                    stall_counter += 1;
                }
            }
            THEN("only one warning is sent when the move finishes") {
                REQUIRE(stall_counter > 1);
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                REQUIRE(test_objs.reporter.messages.size() == 3);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.message_index == 13);
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);
                REQUIRE(err.severity == can::ids::ErrorSeverity::warning);
                auto inc_error_count =
                    std::get<usage_messages::IncreaseErrorCount>(
                        test_objs.reporter.messages[1]);
                std::ignore = inc_error_count;
                Ack ack_msg = std::get<Ack>(test_objs.reporter.messages.back());
                REQUIRE(ack_msg.ack_id ==
                        AckMessageId::complete_without_condition);
                REQUIRE(ack_msg.message_index == 13);
            }
        }
    }

    GIVEN("a move with stall expected stop condition") {
        Move msg1 = Move{.message_index = 101,
                         .duration = 23,
                         .velocity = default_velocity,
                         .stop_condition = static_cast<uint8_t>(Stops::stall)};
        constexpr Move msg2 =
            Move{.duration = 20, .velocity = default_velocity};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 3);
        test_objs.handler.update_move();
        WHEN("encoder doesn't update with the motor") {
            while (test_objs.queue.has_message_isr()) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected but it is expected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN(
                    "all the moves are cleared and only a stopped by condition "
                    "ack message is sent") {
                    REQUIRE(test_objs.queue.get_size() == 0);
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack ack_msg =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(ack_msg.message_index == 101);
                    REQUIRE(ack_msg.ack_id ==
                            AckMessageId::stopped_by_condition);
                }
                THEN("an update motor position message") {
                    auto update_msg =
                        can::messages::UpdateMotorPositionEstimationRequest{
                            .message_index = 123};
                    test_objs.update_position_queue.try_write(update_msg);
                    test_objs.handler.run_interrupt();
                    THEN("a response is received") {
                        REQUIRE(test_objs.reporter.messages.size() == 2);
                        UpdatePositionResponse ack_msg =
                            std::get<UpdatePositionResponse>(
                                test_objs.reporter.messages.back());
                        REQUIRE(ack_msg.message_index == 123);
                    }
                }
            }
        }
    }

    GIVEN("a move with stall expected stop condition with ignore stalls flag") {
        Move msg1 = Move{.message_index = 102,
                         .duration = 23,
                         .velocity = default_velocity,
                         .stop_condition = static_cast<uint8_t>(
                             static_cast<uint8_t>(Stops::stall) ^
                             static_cast<uint8_t>(Stops::ignore_stalls))};
        constexpr Move msg2 =
            Move{.duration = 20, .velocity = default_velocity};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 3);
        test_objs.handler.update_move();
        WHEN("encoder doesn't update with the motor") {
            while (test_objs.queue.has_message_isr()) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected but it is expected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN("the stall is handled instead of ignored") {
                    REQUIRE(test_objs.queue.get_size() == 0);
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack ack_msg =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(ack_msg.message_index == 102);
                    REQUIRE(ack_msg.ack_id ==
                            AckMessageId::stopped_by_condition);
                }
            }
        }
    }

    GIVEN("a home move with a stall detected") {
        test_objs.hw.set_mock_lim_sw(false);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::encoder_position_ok);
        test_objs.hw.position_flags.clear_flag(
            MotorPositionStatus::Flags::stepper_position_ok);
        auto msg1 =
            Move{.duration = 23,
                 .velocity = -default_velocity,
                 .stop_condition = static_cast<uint8_t>(Stops::limit_switch)};
        auto msg2 = Move{.duration = 10, .velocity = default_velocity};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 2);
        test_objs.handler.update_move();

        WHEN("encoder doesn't update with the motor") {
            REQUIRE(test_objs.queue.get_size() == 1);
            REQUIRE(test_objs.reporter.messages.size() == 0);
            for (int i = 0; i < (int)msg1.duration; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN("move completed and no error was raised") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack ack_msg =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(ack_msg.ack_id ==
                            AckMessageId::complete_without_condition);
                }
            }
        }
    }

    GIVEN("a limit switch backoff move with a stall detected") {
        test_objs.hw.set_mock_lim_sw(true);
        auto msg1 = Move{.duration = 23,
                         .velocity = default_velocity,
                         .stop_condition =
                             static_cast<uint8_t>(Stops::limit_switch_backoff)};
        auto msg2 = Move{.duration = 10, .velocity = default_velocity};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 2);
        test_objs.handler.update_move();

        WHEN("encoder doesn't update with the motor") {
            REQUIRE(test_objs.queue.get_size() == 1);
            REQUIRE(test_objs.reporter.messages.size() == 0);
            for (int i = 0; i < (int)msg1.duration; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN("move completed and no error was raised") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    Ack ack_msg =
                        std::get<Ack>(test_objs.reporter.messages.front());
                    REQUIRE(ack_msg.ack_id ==
                            AckMessageId::complete_without_condition);
                }
            }
        }
    }
}