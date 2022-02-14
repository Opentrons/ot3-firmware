#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"
#include "motor-control/core/motor_messages.hpp"

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
 * 1. move message has stop condition limit switch and hits a limit switch- should stop the move with id limit switch
 *
 * 2. move message has stop condition limit switch and finishes the move without hitting a limit switch, should exit with error code
 *
 * 3. move message has non-limit switch stop condition and hits a limit switch - should not finish the move
 *
 * */


TEST_CASE("Move with stop condition == limit switch") {
    HandlerContainer test_objs{};
    Move msg1 = Move{.stop_condition = MoveStopCondition::limit_switch};
    test_objs.queue.try_write_isr(msg1);
    GIVEN("the move is in progress" ) {
        WHEN("the limit switch has been triggered") {
            test_objs.hw.set_mock_lim_sw(true);
            THEN("the move should be stopped with ack id = limit switch") {
                if (test_objs.reporter.messages.size() == 1) {
                    auto msg = test_objs.reporter.messages[0];
                    REQUIRE(msg.ack_id == AckMessageId::triggered_lim_sw);
                }
            }
        }
    }
    GIVEN("the limit switch has not been triggered") {
        test_objs.hw.set_mock_lim_sw(false);
        WHEN("the move is finished") {
            test_objs.handler.finish_current_move();
            THEN("the move should be stopped with ack id = error") {
                if (test_objs.reporter.messages.size() == 2) {
                    auto msg = test_objs.reporter.messages[0];
                    REQUIRE(msg.ack_id == AckMessageId::error);
                }
            }
        }
    }
}
TEST_CASE("Move with stop condition != limit switch") {
    HandlerContainer test_objs{};
    GIVEN("Move with stop condition none in progress"){
        Move msg1 = Move{.stop_condition = MoveStopCondition::none};
        test_objs.queue.try_write_isr(msg1);
        WHEN("a limit switch is triggered") {
            test_objs.hw.set_mock_lim_sw(true);
            THEN("when the move is done, the ack id of the finished message is not lim sw triggered") {
                if (test_objs.reporter.messages.size() == 1) {
                    auto msg = test_objs.reporter.messages[0];
                    REQUIRE(msg.ack_id != AckMessageId::triggered_lim_sw);
                }
            }
        }
    }
}