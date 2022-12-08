#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace motor_handler;

SCENARIO("queue multiple move messages") {
    static constexpr sq0_31 default_velocity = 0x1 << 30;
    GIVEN("a motor interrupt handler") {
        test_mocks::MockMessageQueue<Move> queue;
        test_mocks::MockMoveStatusReporterClient reporter{};
        test_mocks::MockMotorHardware hardware;
        stall_check::StallCheck stall(10, 10, 10);
        auto handler = MotorInterruptHandler(queue, reporter, hardware, stall);

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
                    static_cast<void>(handler.pulse());
                }
                REQUIRE(handler.has_messages() == false);
            }
        }
    }
}
