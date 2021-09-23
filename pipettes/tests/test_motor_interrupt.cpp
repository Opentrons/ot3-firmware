#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "pipettes/tests/mock_message_queue.hpp"

using namespace motor_handler;

SCENARIO("queue multiple move messages") {
    GIVEN("a motor interrupt handler") {
        mock_message_queue::MockMessageQueue<Message> queue();

        static auto handler = MotorInterruptHandler<mock_message_queue::MockMessageQueue>();
        handler.set_message_queue(queue);

        void step_motor();

        void step_motor() {
            if (handler.can_step()) {
                handler.increment_counter();
            } else {
                if (handler.has_messages()) {
                    handler.update_move();
                } else {
                    handler.finish_current_move();
                }
            }
        }

        WHEN("add multiple moves to the queue") {
            THEN("all the moves should exist in order") {
                constexpr Message msg1 = Message{100};
                constexpr Message msg2 = Message{400};
                constexpr Message msg3 = Message{7000};
                constexpr Message msg4 = Message{800};
                queue.try_write(msg1);
                queue.try_write(msg2);
                queue.try_write(msg3);
                queue.try_write(msg4);
//                REQUIRE(ZERO_VALUE == testI2C.stored);
            }
        }

        WHEN("moves have been issued") {
            THEN("the step motor command should execute all of them") {
                while (handler.has_messages()) {
                    step_motor();
                }
                REQUIRE(handler.has_message_isr() == false)
            }
        }

    }

}