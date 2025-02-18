#include <cmath>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace motor_handler;

struct MotorContainer {
    test_mocks::MockMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::Move> queue{};
    test_mocks::MockMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>
        update_position_queue{};
    test_mocks::MockMoveStatusReporterClient reporter{};
    stall_check::StallCheck st{1, 1, 10};
    MotorInterruptHandler<test_mocks::MockMessageQueue,
                          test_mocks::MockMoveStatusReporterClient,
                          motor_messages::Move, test_mocks::MockMotorHardware>
        handler{queue, reporter, hw, st, update_position_queue};
};

SCENARIO("a move is cancelled due to a stop request") {
    MotorContainer test_objs{};

    GIVEN("A few message to home") {
        auto msg = Move{.duration = 500,
                        .velocity = 0x7fffffff,
                        .acceleration = 2,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = static_cast<uint8_t>(
                            MoveStopCondition::limit_switch)};
        test_objs.queue.try_write_isr(msg);
        test_objs.queue.try_write_isr(msg);
        test_objs.queue.try_write_isr(msg);
        WHEN("A cancel request occurs") {
            test_objs.handler.run_interrupt();
            test_objs.handler.run_interrupt();
            test_objs.handler.run_interrupt();
            CHECK(test_objs.hw.steps_taken() == 1);
            test_objs.hw.request_cancel();
            test_objs.handler.run_interrupt();
            THEN("An error and increase error count is sent") {
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::stop_requested);
            }
            THEN("the move is no longer active") {
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.hw.steps_taken() == 1);
            }
            THEN("the queue is emptied") {
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.queue.get_size() == 0);
            }
        }
    }
}

SCENARIO("estop pressed during motor interrupt handler") {
    MotorContainer test_objs{};

    GIVEN("A message to home") {
        auto msg = Move{.duration = 500,
                        .velocity = 0x7fffffff,
                        .acceleration = 2,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = static_cast<uint8_t>(
                            MoveStopCondition::limit_switch)};
        test_objs.queue.try_write_isr(msg);
        test_objs.queue.try_write_isr(msg);
        test_objs.queue.try_write_isr(msg);
        WHEN("Estop is pressed") {
            test_objs.handler.run_interrupt();
            test_objs.handler.run_interrupt();
            test_objs.handler.run_interrupt();
            CHECK(test_objs.hw.steps_taken() == 1);
            test_objs.hw.set_mock_estop_in(true);
            test_objs.handler.run_interrupt();
            THEN("An error and increase error count is sent") {
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::estop_detected);
            }
            THEN("the move is no longer active") {
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.hw.steps_taken() == 1);
            }
            THEN("the queue is emptied") {
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.queue.get_size() == 0);
            }
        }
    }
}

SCENARIO("estop is steady-state pressed") {
    MotorContainer test_objs{};
    test_objs.hw.set_mock_estop_in(true);
    test_objs.handler.run_interrupt();
    test_objs.handler.run_interrupt();
    // estop-active message and increase error count message
    CHECK(test_objs.reporter.messages.size() == 2);
    test_objs.reporter.messages.clear();
    GIVEN("some moves in its queue") {
        auto msg = Move{.message_index = 1,
                        .duration = 500,
                        .velocity = 10,
                        .acceleration = 2,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = static_cast<uint8_t>(
                            MoveStopCondition::limit_switch)};
        test_objs.queue.try_write_isr(msg);
        msg.message_index = 2;
        test_objs.queue.try_write_isr(msg);
        WHEN("the interrupt runs") {
            test_objs.handler.run_interrupt();
            THEN("the first move gets an error") {
                REQUIRE(test_objs.queue.get_size() == 1);
                REQUIRE(test_objs.reporter.messages.size() == 2);
                auto err = std::get<can::messages::ErrorMessage>(
                    test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::estop_detected);
                REQUIRE(err.message_index == 1);
                auto inc_err_cnt = std::get<usage_messages::IncreaseErrorCount>(
                    test_objs.reporter.messages[1]);
                std::ignore = inc_err_cnt;
                AND_WHEN("the interrupt runs again") {
                    test_objs.handler.run_interrupt();
                    THEN("the second move is just ignored") {
                        REQUIRE(test_objs.queue.get_size() == 0);
                        REQUIRE(test_objs.reporter.messages.size() == 2);
                    }
                }
            }
        }
    }
    GIVEN("a position request in its position request queue") {
        auto msg = can::messages::UpdateMotorPositionEstimationRequest{
            .message_index = 123};
        test_objs.update_position_queue.try_write(msg);
        WHEN("the interrupt runs") {
            test_objs.handler.run_interrupt();
            THEN("the position request is handled and responded to") {
                REQUIRE(test_objs.reporter.messages.size() > 0);
                REQUIRE(std::holds_alternative<
                        motor_messages::UpdatePositionResponse>(
                    test_objs.reporter.messages.front()));
            }
        }
    }
}

SCENARIO("negative position reset") {
    MotorContainer test_objs{};

    GIVEN("current encoder position is negative") {
        test_objs.hw.sim_set_encoder_pulses(-1);
        WHEN("correcting negative encoder value") {
            auto ret = test_objs.handler.address_negative_encoder();
            THEN("the encoder count is raised to 0") {
                REQUIRE(ret == 0);
                REQUIRE(test_objs.hw.get_encoder_pulses() == 0);
            }
        }
    }
    GIVEN("current encoder position is positive") {
        int32_t val = GENERATE(0, 100);
        test_objs.hw.sim_set_encoder_pulses(val);
        WHEN("correcting negative encoder value") {
            auto ret = test_objs.handler.address_negative_encoder();
            THEN("the encoder count is not changed") {
                REQUIRE(ret == val);
                REQUIRE(test_objs.hw.get_encoder_pulses() == val);
            }
        }
    }
}

SCENARIO("A safe stop move is sent") {

    MotorContainer test_objs{};

    GIVEN("A message to move, with safe stop") {
        uint64_t duration = 500;
        auto start_vel = 150;
        auto acceleration = -30;
        uint8_t stop_condition = static_cast<uint8_t>(
                            MoveStopCondition::sync_line) | static_cast<uint8_t>(
                            MoveStopCondition::encoder_position_or_safe_stop);
        printf("%x\n", stop_condition);
        auto msg = Move{
                        .message_index=123,
                        .duration = duration,
                        .velocity = start_vel,
                        .acceleration = acceleration,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = stop_condition};
        WHEN("Condition is met") {
            test_objs.hw.sim_set_encoder_pulses(0);
            test_objs.queue.try_write_isr(msg);
            for (auto i = 0; i < int(duration/2); i++) {
                test_objs.hw.sim_set_encoder_pulses(start_vel*i + 0.5*pow((acceleration/1000), 2.0));
                test_objs.handler.run_interrupt();
            }
            test_objs.hw.set_mock_sync_line(true);
            printf("Sync line set\n");
            test_objs.handler.run_interrupt();

            REQUIRE(test_objs.reporter.messages.size() == 1);
            auto resp = std::get<motor_messages::Ack>(
                    test_objs.reporter.messages.front());
            REQUIRE(resp.ack_id == motor_messages::AckMessageId::condition_met);
            REQUIRE(resp.message_index == 123);
            THEN("The move continues") {
                for (auto i = duration/2; i < duration; i++) {
                    test_objs.hw.sim_set_encoder_pulses(start_vel*i + 0.5*pow((acceleration/1000),2.0));
                    test_objs.handler.run_interrupt();
                }
                REQUIRE(test_objs.reporter.messages.size() == 2);
                auto resp = std::get<motor_messages::Ack>(
                        test_objs.reporter.messages[1]);
                REQUIRE(resp.ack_id == motor_messages::AckMessageId::complete_without_condition);
                REQUIRE(resp.message_index == 123);
            }

        }
    }
}
