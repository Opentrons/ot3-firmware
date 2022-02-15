#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace motor_handler;

struct HandlerContainer {
    test_mocks::MockMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::Move> queue{};
    test_mocks::MockMoveStatusReporterClient reporter{};
    MotorInterruptHandler<test_mocks::MockMessageQueue,
                          test_mocks::MockMoveStatusReporterClient>
        handler{queue, reporter, hw};
};

/*test cases:
 * 1. move message has stop condition limit switch and hits a limit switch-
 * should stop the move with id limit switch
 *
 * 2. move message has stop condition limit switch and finishes the move without
 * hitting a limit switch, should exit with error code
 *
 * 3. move message has non-limit switch stop condition and hits a limit switch -
 * should not finish the move
 *
 * */

TEST_CASE("Move with stop condition == limit switch") {
    HandlerContainer test_objs{};
    Move msg1 = Move{.duration = 0.5,
                     .velocity = convert_velocity(0.5),
                     .stop_condition = MoveStopCondition::limit_switch};
    test_objs.queue.try_write_isr(msg1);
    GIVEN("the move is in progress") {
        WHEN("the limit switch has been triggered") {
            test_objs.hw.set_mock_lim_sw(true);
            test_objs.handler.pulse();
            THEN("the move should be stopped with ack id = limit switch") {
                Ack read_ack = test_objs.reporter.messages.back();
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
            }
        }
    }
    GIVEN("the limit switch has not been triggered") {
        test_objs.hw.set_mock_lim_sw(false);
        WHEN("the move is finished") {
            test_objs.handler.pulse();  // check whether I only need to call
                                        // once
            THEN("the move should be stopped with ack id = error") {
                Ack read_ack = test_objs.reporter.messages.back();
                REQUIRE(read_ack.ack_id ==
                        AckMessageId::complete_without_condition);
            }
        }
    }
}
TEST_CASE("Move with stop condition != limit switch") {
    HandlerContainer test_objs{};
    GIVEN("Move with stop condition none in progress") {
        Move msg1 = Move{.duration = 0.5,
                         .velocity = convert_velocity(0.5),
                         .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg1);
        WHEN("a limit switch is triggered") {
            test_objs.hw.set_mock_lim_sw(true);
            THEN(
                "when the move is done, the ack id of the finished message is "
                "not lim sw triggered") {
                test_objs.handler.pulse();
                Ack read_ack = test_objs.reporter.messages.back();
                REQUIRE(read_ack.ack_id !=
                        AckMessageId::complete_without_condition);
            }
        }
    }
}