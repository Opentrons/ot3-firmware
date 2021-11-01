#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

static auto handler =
    MotorInterruptHandler<mock_message_queue::MockMessageQueue,
                          mock_message_queue::MockMessageQueue>();

SCENARIO("queue multiple move messages") {
    static constexpr sq0_31 default_velocity = 0x1 << 30;
    GIVEN("a motor interrupt handler") {
        mock_message_queue::MockMessageQueue<Move> queue;
        mock_message_queue::MockMessageQueue<Ack> completed_queue;

        handler.set_message_queue(&queue, &completed_queue);

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