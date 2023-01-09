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

SCENARIO("estop pressed during motor interrupt handler") {
    MotorContainer test_objs{};

    GIVEN("A message to home") {
        auto msg = Move{.duration = 500,
                        .velocity = 10,
                        .acceleration = 2,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg);
        WHEN("Estop is pressed") {
            test_objs.hw.set_mock_estop_in(true);
            test_objs.handler.run_interrupt();
            THEN("Errors are sent") {
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::estop_detected);

                can::messages::StopRequest stop =
                    std::get<can::messages::StopRequest>(
                        test_objs.reporter.messages.back());
                REQUIRE(stop.message_index == 0);
            }
        }
    }
}
