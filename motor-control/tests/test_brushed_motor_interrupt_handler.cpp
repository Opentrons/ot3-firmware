#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_brushed_motor_components.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace brushed_motor_handler;

auto gear_config = lms::LinearMotionSystemConfig<lms::GearBoxConfig>{
    .mech_config = lms::GearBoxConfig{.gear_diameter = 9},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512,
    .gear_ratio = 84.29};

struct BrushedMotorContainer {
    test_mocks::MockBrushedMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::BrushedMove> queue{};
    test_mocks::MockBrushedMoveStatusReporterClient reporter{};
    test_mocks::MockBrushedMotorDriverIface driver{};
    BrushedMotorInterruptHandler<
        test_mocks::MockMessageQueue,
        test_mocks::MockBrushedMoveStatusReporterClient>
        handler{queue, reporter, hw, driver, gear_config};
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
            test_objs.handler.update_and_start_move();

            THEN("The motor hardware proceeds to grip") {
                /* motor should be gripping */
                REQUIRE(test_objs.hw.get_is_gripping());
                test_objs.hw.set_encoder_value(30000);

                AND_WHEN("The encoder speed timer overflows") {
                    /* this happens when the enc speed is very slow */
                    test_objs.handler.set_enc_idle_state(true);

                    THEN(
                        "Encoder speed tracker is holding off for a 1 ms (32 "
                        "ticks)") {
                        for (uint32_t i = 0; i < HOLDOFF_TICKS; i++) {
                            REQUIRE(!test_objs.handler.is_sensing());
                            REQUIRE(test_objs.reporter.messages.size() == 0);
                            CHECK(test_objs.handler.tick == (i + 1));
                        }

                        AND_THEN("Gripped ack is sent") {
                            CHECK(test_objs.handler.tick == 32);
                            test_objs.handler.run_interrupt();
                            REQUIRE(test_objs.hw.get_encoder_pulses() == 30000);
                            REQUIRE(test_objs.reporter.messages.size() >= 1);
                            Ack read_ack = test_objs.reporter.messages.back();
                            REQUIRE(read_ack.encoder_position == 30000);
                            REQUIRE(read_ack.ack_id ==
                                    AckMessageId::complete_without_condition);
                        }
                    }
                }
            }
        }
    }
    GIVEN("A message to move") {
        auto msg =
            BrushedMove{.duration = 0,
                        .duty_cycle = 0,
                        .group_id = 0,
                        .seq_id = 0,
                        .encoder_position = 610539,  // ~1cm
                        .stop_condition = MoveStopCondition::encoder_position};
        int32_t last_pid_output = test_objs.hw.get_pid_controller_output();
        REQUIRE(last_pid_output == 0.0);
        test_objs.hw.set_encoder_value(0);
        test_objs.queue.try_write_isr(msg);
        WHEN("A brushed move message is received and loaded") {
            test_objs.handler.update_and_start_move();
            THEN("The motor hardware proceeds to move") {
                int32_t i = 0;
                // simulate the motor moving so the pid can update
                while (test_objs.reporter.messages.size() == 0) {
                    // the approxomaite speed of the jaw movenent is 0.55mm/s *
                    // pwm so the distance traveled in one interrupt time is
                    // 0.55mm/s*pwm*0.003s just mulitpy that by
                    // get_encoder_pulses_per_mm to get the encoder delta
                    int32_t encoder_delta =
                        int32_t(test_objs.driver.get_pwm_settings() * 0.55 *
                                (1.0 / 320.0) *
                                gear_config.get_encoder_pulses_per_mm());

                    if (test_objs.hw.get_direction() ==
                        test_mocks::PWM_DIRECTION::positive) {
                        i += encoder_delta;
                    } else {
                        i -= encoder_delta;
                    }

                    test_objs.hw.set_encoder_value(i);
                    test_objs.handler.run_interrupt();
                    if (test_objs.driver.get_pwm_settings() == 0) {
                        break;
                    }
                    REQUIRE((test_objs.driver.get_pwm_settings() > 0 &&
                             test_objs.driver.get_pwm_settings() <= 100));
                }
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.driver.get_pwm_settings() == 0);
                REQUIRE(test_objs.reporter.messages.size() >= 1);
                Ack read_ack = test_objs.reporter.messages.back();
                // check if position is withen acceptable parameters
                REQUIRE(
                    std::abs(read_ack.encoder_position - msg.encoder_position) <
                    int32_t(gear_config.get_encoder_pulses_per_mm() * 0.01));
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
            }
        }
    }
}
