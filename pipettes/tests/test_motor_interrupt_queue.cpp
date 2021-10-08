#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

static auto handler =
    MotorInterruptHandler<mock_message_queue::MockMessageQueue>();

SCENARIO("queue multiple move messages") {
    GIVEN("a motor interrupt handler") {
        mock_message_queue::MockMessageQueue<Move> queue;

        handler.set_message_queue(&queue);

        WHEN("add multiple moves to the queue") {
            THEN("all the moves should exist in order") {
                constexpr Move msg1 = Move{.target_position = 100};
                constexpr Move msg2 = Move{.target_position = 400};
                constexpr Move msg3 = Move{.target_position = 7000};
                constexpr Move msg4 = Move{.target_position = 800};
                queue.try_write(msg1);
                queue.try_write(msg2);
                queue.try_write(msg3);
                queue.try_write(msg4);
                REQUIRE(queue.get_size() == 4);
                REQUIRE(handler.has_messages() == true);
            }
        }

        WHEN("moves have been issued") {
            THEN("the step motor command should execute all of them") {
                while (handler.has_messages()) {
                    handler.pulse();
                }
                REQUIRE(handler.has_messages() == false);
            }
        }
    }
}