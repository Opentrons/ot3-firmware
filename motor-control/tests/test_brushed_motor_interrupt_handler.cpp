#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_brushed_motor_components.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace brushed_motor_handler;

auto gear_config = lms::LinearMotionSystemConfig<lms::GearBoxConfig>{
    .mech_config =
        lms::GearBoxConfig{.gear_diameter = 9, .gear_reduction_ratio = 84.29},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512};

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
            BrushedMove{.duration = 5 * 32000,
                        .duty_cycle = 50,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg);

        WHEN("A brushed move message is received and loaded") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }

            THEN("The motor hardware proceeds to home") {
                /* motor shouldn't be gripping */
                REQUIRE(!test_objs.hw.get_is_gripping());
                test_objs.hw.set_encoder_value(1000);
                test_objs.hw.set_limit_switch(true);

                AND_WHEN("The limit switch is hit") {
                    // Burn through the ticks since they reset with new move
                    for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                        test_objs.handler.run_interrupt();
                    }

                    THEN("Encoder value is reset and homed ack is sent") {
                        REQUIRE(test_objs.hw.get_encoder_pulses() == 0);
                        REQUIRE(test_objs.reporter.messages.size() >= 1);
                        Ack read_ack =
                            std::get<Ack>(test_objs.reporter.messages.back());
                        REQUIRE(read_ack.encoder_position == 0);
                        REQUIRE(read_ack.ack_id ==
                                AckMessageId::stopped_by_condition);
                        REQUIRE(test_objs.handler.is_idle);
                    }
                }
            }
        }
        WHEN("Home message times out") {
            test_objs.reporter.messages.clear();
            while (test_objs.reporter.messages.size() == 0) {
                test_objs.handler.run_interrupt();
            }
            REQUIRE(test_objs.reporter.messages.size() == 1);
            Ack read_ack = std::get<Ack>(test_objs.reporter.messages.back());
            REQUIRE(read_ack.ack_id == AckMessageId::timeout);
        }
    }

    GIVEN("A message to grip") {
        auto msg = BrushedMove{.duration = 5 * 32000,
                               .duty_cycle = 50,
                               .group_id = 0,
                               .seq_id = 0,
                               .stop_condition = MoveStopCondition::none};
        test_objs.queue.try_write_isr(msg);

        WHEN("A brushed move message is received and loaded") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }

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
                            test_objs.handler.run_interrupt();
                            CHECK(test_objs.handler.tick == (i + 1));
                        }

                        AND_THEN("Gripped ack is sent") {
                            CHECK(test_objs.handler.tick == 32);
                            test_objs.handler.run_interrupt();
                            REQUIRE(test_objs.hw.get_encoder_pulses() == 30000);
                            REQUIRE(test_objs.reporter.messages.size() >= 1);
                            Ack read_ack = std::get<Ack>(
                                test_objs.reporter.messages.back());
                            REQUIRE(read_ack.encoder_position == 30000);
                            REQUIRE(read_ack.ack_id ==
                                    AckMessageId::complete_without_condition);
                        }
                    }
                }
            }
        }
        WHEN("grip message times out") {
            // make the gripper think its still moving
            test_objs.handler.set_enc_idle_state(false);
            test_objs.reporter.messages.clear();
            while (test_objs.reporter.messages.size() == 0) {
                test_objs.handler.run_interrupt();
            }
            REQUIRE(test_objs.reporter.messages.size() == 1);
            Ack read_ack = std::get<Ack>(test_objs.reporter.messages.back());
            REQUIRE(read_ack.ack_id == AckMessageId::timeout);
        }
    }
    GIVEN("A message to move") {
        auto msg =
            BrushedMove{.duration = 5 * 32000,
                        .duty_cycle = 0,
                        .group_id = 0,
                        .seq_id = 0,
                        .encoder_position = 61054,  // ~1cm
                        .stop_condition = MoveStopCondition::encoder_position};
        int32_t last_pid_output = test_objs.hw.get_pid_controller_output();
        REQUIRE(last_pid_output == 0.0);
        test_objs.hw.set_encoder_value(0);
        test_objs.queue.try_write_isr(msg);
        WHEN("A brushed move message is received and loaded") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }
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
                                (1.0 / 32000.0) *
                                gear_config.get_encoder_pulses_per_mm());
                    // when the encoder delta is very small the int can get
                    // stuck at 0 when it should be like 0.9 at the very
                    // end of the move. so floor it 1 if the pwm is > 0 at all
                    if (test_objs.driver.get_pwm_settings() > 0) {
                        encoder_delta =
                            std::clamp(encoder_delta, 1,
                                       std::numeric_limits<int32_t>::max());
                    }
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
                    REQUIRE(test_objs.driver.get_pwm_settings() <= 100);
                }
                test_objs.handler.run_interrupt();
                REQUIRE(test_objs.driver.get_pwm_settings() == 0);
                REQUIRE(test_objs.reporter.messages.size() >= 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());
                // check if position is withen acceptable parameters
                REQUIRE(
                    std::abs(read_ack.encoder_position - msg.encoder_position) <
                    int32_t(gear_config.get_encoder_pulses_per_mm() * 0.01));
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
            }
        }
        WHEN("move message times out") {
            test_objs.reporter.messages.clear();
            while (test_objs.reporter.messages.size() == 0) {
                test_objs.handler.run_interrupt();
            }
            REQUIRE(test_objs.reporter.messages.size() == 1);
            Ack read_ack = std::get<Ack>(test_objs.reporter.messages.back());
            REQUIRE(read_ack.ack_id == AckMessageId::timeout);
        }
    }
}

SCENARIO("estop pressed during Brushed motor interrupt handler") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to home") {
        auto msg =
            BrushedMove{.duration = 5 * 32000,
                        .duty_cycle = 50,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg);
        WHEN("Estop is pressed") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }
            test_objs.hw.set_estop_in(true);
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

SCENARIO("labware dropped during grip move") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to grip") {
        auto msg = BrushedMove{.duration = 5 * 32000,
                               .duty_cycle = 50,
                               .group_id = 0,
                               .seq_id = 0,
                               .stop_condition = MoveStopCondition::none};
        test_objs.queue.try_write_isr(msg);
        WHEN("grip is complete") {
            test_objs.hw.set_encoder_value(1200);
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= 100; i++) {
                test_objs.handler.run_interrupt();
            }
            test_objs.handler.set_enc_idle_state(true);
            test_objs.handler.run_interrupt();
            THEN("Move complete message is sent") {
                REQUIRE(test_objs.reporter.messages.size() == 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());

                REQUIRE(read_ack.ack_id ==
                        AckMessageId::complete_without_condition);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                test_objs.hw.set_encoder_value(1800);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                    test_objs.handler.run_interrupt();
                }
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::labware_dropped);

                can::messages::StopRequest stop =
                    std::get<can::messages::StopRequest>(
                        test_objs.reporter.messages.back());
                REQUIRE(stop.message_index == 0);
                REQUIRE(test_objs.hw.get_stay_enabled() == true);
            }
        }
    }
}

SCENARIO("collision while homed") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to home") {
        auto msg =
            BrushedMove{.duration = 5 * 32000,
                        .duty_cycle = 50,
                        .group_id = 0,
                        .seq_id = 0,
                        .stop_condition = MoveStopCondition::limit_switch};
        test_objs.queue.try_write_isr(msg);
        WHEN("home is complete") {
            test_objs.hw.set_encoder_value(1200);
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= 100; i++) {
                test_objs.handler.run_interrupt();
            }
            test_objs.hw.set_limit_switch(true);
            test_objs.handler.set_enc_idle_state(true);
            test_objs.handler.run_interrupt();
            THEN("Encoder value is reset and homed ack is sent") {
                REQUIRE(test_objs.hw.get_encoder_pulses() == 0);
                REQUIRE(test_objs.reporter.messages.size() >= 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());
                REQUIRE(read_ack.encoder_position == 0);
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
                REQUIRE(test_objs.handler.is_idle);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                test_objs.hw.set_encoder_value(200);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                    test_objs.handler.run_interrupt();
                }
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);

                can::messages::StopRequest stop =
                    std::get<can::messages::StopRequest>(
                        test_objs.reporter.messages.back());
                REQUIRE(stop.message_index == 0);
                REQUIRE(test_objs.hw.get_stay_enabled() == false);
            }
        }
    }
}

SCENARIO("A collision during position controlled move") {
    BrushedMotorContainer test_objs{};
    auto msg =
        BrushedMove{.duration = 5 * 32000,
                    .duty_cycle = 0,
                    .group_id = 0,
                    .seq_id = 0,
                    .encoder_position = 61054,  // ~1cm
                    .stop_condition = MoveStopCondition::encoder_position};
    int32_t last_pid_output = test_objs.hw.get_pid_controller_output();
    REQUIRE(last_pid_output == 0.0);
    test_objs.hw.set_encoder_value(0);
    test_objs.queue.try_write_isr(msg);
    WHEN("A brushed move message is received and loaded") {
        // Burn through the startup ticks
        for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
            test_objs.handler.run_interrupt();
        }
        THEN("The motor hardware proceeds to move") {
            int32_t i = 0;
            // simulate the motor moving so the pid can update
            while (test_objs.reporter.messages.size() == 0) {
                // the approxomaite speed of the jaw movenent is 0.55mm/s *
                // pwm so the distance traveled in one interrupt time is
                // 0.55mm/s*pwm*0.003s just mulitpy that by
                // get_encoder_pulses_per_mm to get the encoder delta
                int32_t encoder_delta = int32_t(
                    test_objs.driver.get_pwm_settings() * 0.55 *
                    (1.0 / 32000.0) * gear_config.get_encoder_pulses_per_mm());
                // when the encoder delta is very small the int can get
                // stuck at 0 when it should be like 0.9 at the very
                // end of the move. so floor it 1 if the pwm is > 0 at all
                if (test_objs.driver.get_pwm_settings() > 0) {
                    encoder_delta = std::clamp(
                        encoder_delta, 1, std::numeric_limits<int32_t>::max());
                }
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
                REQUIRE(test_objs.driver.get_pwm_settings() <= 100);
            }
            test_objs.handler.run_interrupt();
            REQUIRE(test_objs.driver.get_pwm_settings() == 0);
            THEN("Move complete message is sent") {
                REQUIRE(test_objs.reporter.messages.size() >= 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());
                // check if position is withen acceptable parameters
                REQUIRE(
                    std::abs(read_ack.encoder_position - msg.encoder_position) <
                    int32_t(gear_config.get_encoder_pulses_per_mm() * 0.01));
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                test_objs.hw.set_encoder_value(200000);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                    test_objs.handler.run_interrupt();
                }
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);

                can::messages::StopRequest stop =
                    std::get<can::messages::StopRequest>(
                        test_objs.reporter.messages.back());
                REQUIRE(stop.message_index == 0);
                REQUIRE(test_objs.hw.get_stay_enabled() == false);
            }
        }
    }
}
