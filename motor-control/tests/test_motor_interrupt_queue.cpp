#include "can/core/ids.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"
#include "motor-control/tests/mock_motor_driver_client.hpp"

using namespace motor_handler;

TEST_CASE("motor interrupt handler queue functionality") {
    static constexpr sq0_31 default_velocity = 0x1 << 30;
    GIVEN("a motor interrupt handler") {
        test_mocks::MockMessageQueue<Move> queue;
        test_mocks::MockMessageQueue<
            can::messages::UpdateMotorPositionEstimationRequest>
            update_position_queue;
        test_mocks::MockMoveStatusReporterClient reporter{};
        test_mocks::MockMotorDriverClient driver{};
        test_mocks::MockMotorHardware hardware;
        stall_check::StallCheck stall(10, 10, 10);
        auto handler = MotorInterruptHandler(queue, reporter, driver, hardware, stall,
                                             update_position_queue);

        WHEN("add multiple moves to the queue") {
            THEN("all the moves should exist in order") {
                constexpr Move msg1 =
                    Move{.duration = 100, .velocity = default_velocity};
                constexpr Move msg2 =
                    Move{.duration = 400, .velocity = default_velocity};
                constexpr Move msg3 =
                    Move{.duration = 7000, .velocity = default_velocity};
                constexpr Move msg4 =
                    Move{.duration = 800, .velocity = default_velocity};
                queue.try_write(msg1);
                queue.try_write(msg2);
                queue.try_write(msg3);
                queue.try_write(msg4);
                REQUIRE(queue.get_size() == 4);
                REQUIRE(handler.has_move_messages() == true);

                WHEN("moves have been issued") {
                    THEN("the step motor command should execute all of them") {
                        while (handler.has_move_messages()) {
                            static_cast<void>(handler.run_interrupt());
                        }
                        REQUIRE(handler.has_move_messages() == false);
                    }
                }
                WHEN("a move is updated") {
                    CHECK(msg1.start_encoder_position == 0);
                    hardware.sim_set_encoder_pulses(300);
                    THEN(
                        "the start encoder position should be updated to "
                        "buffered move") {
                        handler.update_move();
                        REQUIRE(handler.get_buffered_move()
                                    .start_encoder_position == 300);
                    }
                }
                WHEN("a movement stalls") {
                    handler.run_interrupt();
                    handler.cancel_and_clear_moves(
                        can::ids::ErrorCode::hardware,
                        can::ids::ErrorSeverity::warning);
                    THEN("the other pending movements are cancelled") {
                        REQUIRE(handler.has_move_messages());
                        // There are 3 more messages to clear
                        for (auto i = 0; i < 3; ++i) {
                            handler.run_interrupt();
                        }
                        REQUIRE(!handler.has_move_messages());
                        AND_WHEN("more moves are enqueued") {
                            queue.try_write(msg1);
                            queue.try_write(msg2);
                            THEN("the move is executed normallly") {
                                for (auto i = 0; i < 3; ++i) {
                                    handler.run_interrupt();
                                }
                                REQUIRE(handler.has_move_messages());
                            }
                        }
                    }
                }
            }
        }

        WHEN("enqueuing an UpdateMotorPositionEstimationRequest") {
            // Test both good & bad situations
            bool should_change = GENERATE(true, false);
            if (should_change) {
                hardware.position_flags.set_flag(
                    MotorPositionStatus::Flags::encoder_position_ok);
            }
            REQUIRE(stall.encoder_ticks_to_stepper_ticks(1234) == 1234);
            hardware.sim_set_encoder_pulses(1234);
            handler.set_current_position(100);
            auto msg = can::messages::UpdateMotorPositionEstimationRequest{
                .message_index = 123};
            update_position_queue.try_write(msg);
            AND_WHEN("running the interrupt") {
                handler.run_interrupt();
                THEN("the message is consumed") {
                    REQUIRE(!update_position_queue.has_message());
                }
                THEN("a response is sent to the reporter") {
                    REQUIRE(reporter.messages.size() > 0);
                    REQUIRE(std::holds_alternative<
                            motor_messages::UpdatePositionResponse>(
                        reporter.messages.front()));
                    auto response =
                        std::get<motor_messages::UpdatePositionResponse>(
                            reporter.messages.front());
                    REQUIRE(response.encoder_pulses == 1234);
                    REQUIRE((response.stepper_position_counts == 1234) ==
                            should_change);
                    REQUIRE(response.message_index == msg.message_index);
                }
                THEN("the stepper position is updated") {
                    REQUIRE(
                        (hardware.position_flags.check_flag(
                            MotorPositionStatus::Flags::stepper_position_ok)) ==
                        should_change);
                    REQUIRE((hardware.get_step_tracker() == 1234) ==
                            should_change);
                }
            }
        }
        GIVEN("negative encoder position") {
            hardware.sim_set_encoder_pulses(-1);
            hardware.position_flags.set_flag(
                MotorPositionStatus::Flags::encoder_position_ok);
            WHEN("enqueuing an UpdateMotorPositionEstimationRequest") {
                auto msg = can::messages::UpdateMotorPositionEstimationRequest{
                    .message_index = 555};
                update_position_queue.try_write(msg);
                AND_WHEN("running the interrupt") {
                    handler.run_interrupt();
                    THEN("the encoder value is reset to 0") {
                        REQUIRE(hardware.get_encoder_pulses() == 0);
                    }
                    THEN("the response has the encoder reset to 0") {
                        auto response =
                            std::get<motor_messages::UpdatePositionResponse>(
                                reporter.messages.front());
                        REQUIRE(response.encoder_pulses == 0);
                        REQUIRE(response.stepper_position_counts == 0);
                        REQUIRE(response.message_index == msg.message_index);
                    }
                }
            }
        }
        WHEN(
            "enqueuing a move message AND an "
            "UpdateMotorPositionEstimationRequest") {
            constexpr Move move_msg =
                Move{.duration = 100, .velocity = default_velocity};
            auto update_msg =
                can::messages::UpdateMotorPositionEstimationRequest{
                    .message_index = 123};
            update_position_queue.try_write(update_msg);
            queue.try_write(move_msg);
            THEN("update message is ACK'd with an error") {
                static_cast<void>(handler.run_interrupt());
                REQUIRE(reporter.messages.size() > 0);
                REQUIRE(std::holds_alternative<can::messages::ErrorMessage>(
                    reporter.messages.front()));
                auto response = std::get<can::messages::ErrorMessage>(
                    reporter.messages.front());
                REQUIRE(response.message_index == 123);
                REQUIRE(response.error_code == can::ids::ErrorCode::motor_busy);
                REQUIRE(response.severity == can::ids::ErrorSeverity::warning);
            }
        }
    }
}
