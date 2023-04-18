#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

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
    stall_check::StallCheck stall{tick_per_um, tick_per_um, stall_threshold_um};
    MotorInterruptHandler<test_mocks::MockMessageQueue,
                          test_mocks::MockMoveStatusReporterClient, Move,
                          test_mocks::MockMotorHardware>
        handler{queue, reporter, hw, stall, update_position_queue};
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
        auto msg1 = Move{.duration = 23,
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
                THEN(
                    "a unrecoverable collision error is raised and a stop "
                    "request is sent") {
                    REQUIRE(test_objs.reporter.messages.size() == 2);
                    can::messages::ErrorMessage err =
                        std::get<can::messages::ErrorMessage>(
                            test_objs.reporter.messages.front());
                    REQUIRE(err.error_code ==
                            can::ids::ErrorCode::collision_detected);
                    REQUIRE(err.severity ==
                            can::ids::ErrorSeverity::unrecoverable);
                    can::messages::StopRequest stop =
                        std::get<can::messages::StopRequest>(
                            test_objs.reporter.messages.back());
                    REQUIRE(stop.message_index == 0);
                }
            }
        }
    }

    GIVEN("a move with ignore_stalls stop condition") {
        auto cond =
            static_cast<uint8_t>(GENERATE(Stops::none, Stops::sync_line));
        auto msg1 =
            Move{.message_index = 13,
                 .duration = 30,
                 .velocity = default_velocity,
                 .stop_condition = static_cast<uint8_t>(
                     static_cast<uint8_t>(Stops::ignore_stalls) ^ cond)};
        auto msg2 =
            Move{.duration = 10,
                 .velocity = default_velocity,
                 .stop_condition = static_cast<uint8_t>(
                     static_cast<uint8_t>(Stops::ignore_stalls) ^ cond)};
        test_objs.queue.try_write(msg1);
        test_objs.queue.try_write(msg2);
        REQUIRE(test_objs.queue.get_size() == 2);
        test_objs.handler.update_move();
        WHEN("encoder doesn't update with the motor") {
            REQUIRE(test_objs.queue.get_size() == 1);
            REQUIRE(test_objs.reporter.messages.size() == 0);
            for (int i = 0; i < (int)msg1.duration + 1; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN(
                "the stall is detected but it is ignored, and the move is "
                "completed") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                REQUIRE(test_objs.reporter.messages.size() == 1);
                Ack ack_msg =
                    std::get<Ack>(test_objs.reporter.messages.front());
                REQUIRE(ack_msg.ack_id ==
                        AckMessageId::complete_without_condition);
                REQUIRE(ack_msg.message_index == 13);
                AND_WHEN("the interrupt runs again") {
                    for (int i = 0; i < (int)msg2.duration; ++i) {
                        test_objs.handler.run_interrupt();
                    }
                    THEN("the second move will be executed as normal") {
                        REQUIRE(test_objs.reporter.messages.size() == 2);
                        Ack ack_msg =
                            std::get<Ack>(test_objs.reporter.messages.back());
                        REQUIRE(ack_msg.ack_id ==
                                AckMessageId::complete_without_condition);
                    }
                }
            }
        }
    }

    GIVEN("a move with stall expected stop condition") {
        Move msg1 = Move{.duration = 23,
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
            for (int i = 0; i < (int)msg1.duration; ++i) {
                test_objs.handler.run_interrupt();
            }
            THEN("the stall is detected but it is expected") {
                REQUIRE(!test_objs.hw.position_flags.check_flag(
                    Flags::stepper_position_ok));
                THEN(
                    "a recoverable collision error is raised, the next move is "
                    "discarded") {
                    REQUIRE(test_objs.reporter.messages.size() == 1);
                    can::messages::ErrorMessage err =
                        std::get<can::messages::ErrorMessage>(
                            test_objs.reporter.messages.front());
                    REQUIRE(err.error_code ==
                            can::ids::ErrorCode::collision_detected);
                    REQUIRE(err.severity ==
                            can::ids::ErrorSeverity::recoverable);
                    THEN("the rest of the move queue will be cleared") {
                        REQUIRE(test_objs.handler.clear_queue_until_empty);
                    }
                }
            }
        }
    }
}