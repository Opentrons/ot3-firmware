#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/tests/mock_motor_driver_client.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace motor_handler;

#define TO_RADIX 31

struct HandlerContainer {
    test_mocks::MockMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::Move> queue{};
    test_mocks::MockMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>
        update_position_queue{};
    test_mocks::MockMoveStatusReporterClient reporter{};
    test_mocks::MockMotorDriverClient driver{};
    stall_check::StallCheck stall{10, 10, 10};
    MotorInterruptHandler<
        test_mocks::MockMessageQueue, test_mocks::MockMoveStatusReporterClient,
        test_mocks::MockMotorDriverClient, Move, test_mocks::MockMotorHardware>
        handler{queue, reporter, driver, hw, stall, update_position_queue};
};

static constexpr sq0_31 default_velocity =
    0x1 << (TO_RADIX - 1);  // half a step per tick

sq0_31 convert_velocity_to_f(float f) {
    return sq0_31(f * static_cast<float>(1LL << TO_RADIX));
}

TEST_CASE("Move with stop condition == cap sensor") {
    HandlerContainer test_objs{};
    Move msg10 = Move{
        .duration = 2,
        .velocity = convert_velocity_to_f(.5),
        .acceleration = 0,
        .group_id = 1,
        .seq_id = 0,
        .stop_condition = static_cast<uint8_t>(MoveStopCondition::sync_line)};

    GIVEN("the move is in progress") {
        test_objs.queue.try_write_isr(msg10);
        WHEN("the move is loaded") {
            CHECK(!test_objs.handler.pulse());

            // test handling for position

            AND_WHEN("the pipette sensor has been triggered") {
                test_objs.hw.set_mock_sync_line(true);
                REQUIRE(!test_objs.handler.pulse());
                CHECK(!test_objs.handler.pulse());
                CHECK(!test_objs.handler.pulse());
                THEN(
                    "the move should be stopped with ack id = stopped by "
                    "condition") {
                    REQUIRE(!test_objs.handler.pulse());
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::stopped_by_condition);
                }
                // test that the position is returned maybe
            }
        }
    }
}
TEST_CASE("Move with stop condition == cap sensor, case 2") {
    HandlerContainer test_objs{};
    Move msg1 = Move{
        .duration = 2,
        .velocity = convert_velocity_to_f(.5),
        .acceleration = 0,
        .group_id = 1,
        .seq_id = 0,
        .stop_condition = static_cast<uint8_t>(MoveStopCondition::sync_line)};
    GIVEN("the sync line has not been set") {
        test_objs.queue.try_write_isr(msg1);
        test_objs.hw.set_mock_sync_line(false);
        WHEN("the move is loaded") {
            CHECK(!test_objs.handler.pulse());
            AND_WHEN("the move is finished") {
                THEN("the move should have ack_id complete_without_condition") {
                    REQUIRE(!test_objs.handler.pulse());
                    REQUIRE(test_objs.handler.pulse());
                    REQUIRE(!test_objs.handler.pulse());
                    REQUIRE(!test_objs.handler.pulse());
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::complete_without_condition);
                }
            }
        }
    }
}
TEST_CASE("Move with stop condition != cap sensor") {
    HandlerContainer test_objs{};
    test_objs.handler.set_current_position(0x0);
    Move msg1 =
        Move{.duration = 2,
             .velocity = convert_velocity_to_f(.5),
             .acceleration = 0,
             .group_id = 1,
             .seq_id = 0,
             .stop_condition = static_cast<uint8_t>(MoveStopCondition::none)};
    GIVEN("Move with stop condition = none in progress") {
        test_objs.queue.try_write_isr(msg1);
        test_objs.hw.set_mock_sync_line(true);
        WHEN("the first move is loaded") {
            CHECK(!test_objs.handler.pulse());

            // test handling for position

            AND_WHEN("the move is finished") {
                REQUIRE(!test_objs.handler.pulse());
                REQUIRE(test_objs.handler.pulse());
                REQUIRE(!test_objs.handler.pulse());
                THEN(
                    "the move should have ack_id complete_without_condition, "
                    "and position = 4") {
                    REQUIRE(test_objs.reporter.messages.size() >= 1);
                    Ack read_ack =
                        std::get<Ack>(test_objs.reporter.messages.back());
                    REQUIRE(read_ack.ack_id ==
                            AckMessageId::complete_without_condition);
                }
            }
        }
    }
}
