#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_brushed_motor_components.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace brushed_motor_handler;

struct BrushedMotorContainer {
    test_mocks::MockBrushedMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::BrushedMove> queue{};
    test_mocks::MockBrushedMoveStatusReporterClient reporter{};
    test_mocks::MockBrushedMotorDriverIface driver{};
    BrushedMotorInterruptHandler<
        test_mocks::MockMessageQueue,
        test_mocks::MockBrushedMoveStatusReporterClient>
        handler{queue, reporter, hw, driver};
};

SCENARIO("Brushed motor interrupt handler handle move messages") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to home") {
        auto msg =
            BrushedMove{.duration = 0,
                        .duty_cycle = 50,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg);

        WHEN("A brushed move message is received and loaded") {
            test_objs.handler.run_interrupt();

            THEN("The motor hardware proceeds to home") {
                /* handler shouldn't be idle */
                REQUIRE(!test_objs.handler.is_idle);
                /* motor shouldn't be gripping */
                REQUIRE(!test_objs.hw.get_is_gripping());
                test_objs.hw.set_encoder_value(1000);
                test_objs.hw.set_limit_switch(true);

                AND_WHEN("The limit switch is hit") {
                    test_objs.handler.run_interrupt();

                    THEN("Encoder value is reset and homed ack is sent") {
                        REQUIRE(test_objs.hw.get_encoder_pulses() == 0);
                        REQUIRE(test_objs.reporter.messages.size() >= 1);
                        Ack read_ack = test_objs.reporter.messages.back();
                        REQUIRE(read_ack.encoder_position == 0);
                        REQUIRE(read_ack.ack_id ==
                                AckMessageId::stopped_by_condition);
                        REQUIRE(test_objs.handler.is_idle);
                    }
                }
            }
        }
    }

    GIVEN("A message to grip") {
        auto msg = BrushedMove{.duration = 0,
                               .duty_cycle = 50,
                               .group_id = 0,
                               .seq_id = 0,
                               .stop_condition = MoveStopCondition::none};
        test_objs.queue.try_write_isr(msg);

        WHEN("A brushed move message is received and loaded") {
            test_objs.handler.run_interrupt();

            THEN("The motor hardware proceeds to grip") {
                /* handler shouldn't be idle */
                REQUIRE(!test_objs.handler.is_idle);
                /* motor should be gripping */
                REQUIRE(test_objs.hw.get_is_gripping());
                test_objs.hw.set_encoder_value(30000);

                AND_WHEN("The encoder speed timer overflows") {
                    /* this happens when the enc speed is very slow */
                    test_objs.handler.enc_speed_timer_overflows();
                    test_objs.handler.run_interrupt();

                    THEN("Gripped ack is sent") {
                        REQUIRE(test_objs.hw.get_encoder_pulses() == 30000);
                        REQUIRE(test_objs.reporter.messages.size() >= 1);
                        Ack read_ack = test_objs.reporter.messages.back();
                        REQUIRE(read_ack.encoder_position == 30000);
                        REQUIRE(read_ack.ack_id ==
                                AckMessageId::complete_without_condition);
                        REQUIRE(test_objs.handler.is_idle);
                    }
                }
            }
        }
    }
}