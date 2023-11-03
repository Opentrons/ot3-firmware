#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
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
auto error_config =
    error_tolerance_config::BrushedMotorErrorTolerance{gear_config};

auto constexpr home_msg =
    BrushedMove{.duration = 5 * 32000,
                .duty_cycle = 50,
                .group_id = 0,
                .seq_id = 0,
                .stay_engaged = 0,
                .stop_condition = MoveStopCondition::limit_switch};

auto constexpr grip_msg =
    BrushedMove{.duration = 5 * 32000,
                .duty_cycle = 50,
                .group_id = 0,
                .seq_id = 0,
                .stay_engaged = 1,
                .stop_condition = MoveStopCondition::none};

auto constexpr move_msg =
    BrushedMove{.duration = 5 * 32000,
                .duty_cycle = 0,
                .group_id = 0,
                .seq_id = 0,
                .encoder_position = 61054,  // ~1cm
                .stop_condition = MoveStopCondition::encoder_position};

struct BrushedMotorContainer {
    test_mocks::MockBrushedMotorHardware hw{};
    test_mocks::MockMessageQueue<motor_messages::BrushedMove> queue{};
    test_mocks::MockBrushedMoveStatusReporterClient reporter{};
    test_mocks::MockBrushedMotorDriverIface driver{};
    BrushedMotorInterruptHandler<
        test_mocks::MockMessageQueue,
        test_mocks::MockBrushedMoveStatusReporterClient>
        handler{queue, reporter, hw, driver, error_config};
};

SCENARIO("Brushed motor interrupt handler handle move messages") {
    BrushedMotorContainer test_objs{};
    GIVEN("A message to home") {
        test_objs.queue.try_write_isr(home_msg);

        WHEN("A brushed move message is received and loaded") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }

            THEN("The motor hardware proceeds to home") {
                /* motor shouldn't be gripping */
                REQUIRE(!test_objs.hw.get_is_gripping());
                REQUIRE(!test_objs.hw.get_stay_enabled());
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
                        REQUIRE(!test_objs.hw.get_stay_enabled());
                        REQUIRE(test_objs.hw.get_motor_state() ==
                                BrushedMotorState::FORCE_CONTROLLING_HOME);
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
            REQUIRE(!test_objs.hw.get_stay_enabled());
            REQUIRE(test_objs.hw.get_motor_state() ==
                    BrushedMotorState::FORCE_CONTROLLING_HOME);
        }
    }

    GIVEN("A message to grip") {
        test_objs.queue.try_write_isr(grip_msg);

        WHEN("A brushed move message is received and loaded") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }

            THEN("The motor hardware proceeds to grip") {
                /* motor should be gripping */
                REQUIRE(test_objs.hw.get_is_gripping());
                REQUIRE(test_objs.hw.get_stay_enabled());
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
                            REQUIRE(test_objs.hw.get_stay_enabled());
                            REQUIRE(test_objs.hw.get_motor_state() ==
                                    BrushedMotorState::FORCE_CONTROLLING);
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
            REQUIRE(test_objs.hw.get_stay_enabled());
            REQUIRE(test_objs.hw.get_motor_state() ==
                    BrushedMotorState::FORCE_CONTROLLING);
        }
    }
    GIVEN("A message to move") {
        int32_t last_pid_output = test_objs.hw.get_pid_controller_output();
        REQUIRE(last_pid_output == 0.0);
        test_objs.hw.set_encoder_value(0);
        test_objs.queue.try_write_isr(move_msg);
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
                REQUIRE(std::abs(read_ack.encoder_position -
                                 move_msg.encoder_position) <
                        int32_t(gear_config.get_encoder_pulses_per_mm() * 2));
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
                REQUIRE(test_objs.hw.get_motor_state() ==
                        BrushedMotorState::POSITION_CONTROLLING);
                REQUIRE(!test_objs.hw.get_stay_enabled());
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
            REQUIRE(test_objs.hw.get_motor_state() ==
                    BrushedMotorState::POSITION_CONTROLLING);
            REQUIRE(!test_objs.hw.get_stay_enabled());
        }
    }
}

SCENARIO("estop pressed during Brushed motor interrupt handler") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to grip") {
        test_objs.queue.try_write_isr(grip_msg);
        WHEN("Estop is pressed") {
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                test_objs.handler.run_interrupt();
            }
            REQUIRE(!test_objs.handler.in_estop);
            REQUIRE(test_objs.hw.get_stay_enabled());
            test_objs.hw.set_estop_in(true);
            test_objs.handler.run_interrupt();
            THEN("Errors are sent") {
                // An error and increase error count is sent
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::estop_detected);
                THEN(
                    "motor should stay engaged and motor state should "
                    "persist") {
                    REQUIRE(test_objs.handler.in_estop);
                    REQUIRE(test_objs.hw.get_stay_enabled());
                    REQUIRE(test_objs.hw.get_motor_state() ==
                            BrushedMotorState::FORCE_CONTROLLING);
                }
            }
        }
    }
}

SCENARIO("labware dropped during grip move") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to grip") {
        test_objs.queue.try_write_isr(grip_msg);
        WHEN("grip is complete") {
            test_objs.hw.set_encoder_value(1200);
            // Burn through the startup ticks
            for (uint32_t i = 0; i <= 100; i++) {
                test_objs.handler.run_interrupt();
            }
            REQUIRE(test_objs.hw.get_stay_enabled());
            test_objs.handler.set_enc_idle_state(true);
            test_objs.handler.run_interrupt();
            THEN("Move complete message is sent") {
                REQUIRE(test_objs.reporter.messages.size() == 1);
                Ack read_ack =
                    std::get<Ack>(test_objs.reporter.messages.back());

                REQUIRE(read_ack.ack_id ==
                        AckMessageId::complete_without_condition);
                REQUIRE(!test_objs.handler.error_handled);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                test_objs.hw.set_encoder_value(40000);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                    test_objs.handler.run_interrupt();
                }
                // An error and increase error count is sent
                REQUIRE(test_objs.reporter.messages.size() == 3);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::labware_dropped);
                REQUIRE(test_objs.handler.error_handled);
                REQUIRE(test_objs.hw.get_stay_enabled());
                // position reported
                motor_messages::UpdatePositionResponse pos_response =
                    std::get<motor_messages::UpdatePositionResponse>(
                        test_objs.reporter.messages.back());
                REQUIRE(pos_response.encoder_pulses == 40000);
            }
        }
    }
}

SCENARIO("collision while homed") {
    BrushedMotorContainer test_objs{};

    GIVEN("A message to home") {
        test_objs.queue.try_write_isr(home_msg);
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
                REQUIRE(!test_objs.handler.error_handled);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                test_objs.hw.set_encoder_value(40000);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS; i++) {
                    test_objs.handler.run_interrupt();
                }
                // An error and increase error count is sent
                REQUIRE(test_objs.reporter.messages.size() == 3);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);
                REQUIRE(test_objs.handler.error_handled);
                REQUIRE(!test_objs.hw.get_stay_enabled());
                // position reported
                motor_messages::UpdatePositionResponse pos_response =
                    std::get<motor_messages::UpdatePositionResponse>(
                        test_objs.reporter.messages.back());
                REQUIRE(pos_response.encoder_pulses == 40000);
            }
        }
    }
}

SCENARIO("A collision during position controlled move") {
    BrushedMotorContainer test_objs{};
    int32_t last_pid_output = test_objs.hw.get_pid_controller_output();
    REQUIRE(last_pid_output == 0.0);
    test_objs.hw.set_encoder_value(0);
    test_objs.queue.try_write_isr(move_msg);
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
                REQUIRE(std::abs(read_ack.encoder_position -
                                 move_msg.encoder_position) <
                        int32_t(gear_config.get_encoder_pulses_per_mm() * 2));
                REQUIRE(read_ack.ack_id == AckMessageId::stopped_by_condition);
                REQUIRE(!test_objs.handler.error_handled);
            }
            test_objs.reporter.messages.clear();
            THEN("Movement starts again") {
                REQUIRE(!test_objs.handler.has_active_move());
                test_objs.hw.set_encoder_value(200000);
                test_objs.handler.set_enc_idle_state(false);
                // Burn through the 2 x holdoff ticks
                for (uint32_t i = 0; i <= HOLDOFF_TICKS * 2; i++) {
                    test_objs.handler.run_interrupt();
                }
                // An error and increase error count is sent
                REQUIRE(test_objs.reporter.messages.size() == 3);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code ==
                        can::ids::ErrorCode::collision_detected);
                REQUIRE(test_objs.handler.error_handled);
                REQUIRE(!test_objs.hw.get_stay_enabled());
                // position reported
                motor_messages::UpdatePositionResponse pos_response =
                    std::get<motor_messages::UpdatePositionResponse>(
                        test_objs.reporter.messages.back());
                REQUIRE(pos_response.encoder_pulses == 200000);
            }
        }
    }
}

SCENARIO("handler encounter error during idle move") {
    BrushedMotorContainer test_objs{};
    auto og_motor_state = GENERATE(BrushedMotorState::FORCE_CONTROLLING_HOME,
                                   BrushedMotorState::FORCE_CONTROLLING,
                                   BrushedMotorState::POSITION_CONTROLLING);

    GIVEN("the encoder value begins to error during an idle move") {
        test_objs.hw.set_motor_state(og_motor_state);
        test_objs.hw.set_encoder_value(200000);
        test_objs.handler.set_enc_idle_state(false);

        WHEN("the first idle move is executed and an error is detected") {
            test_objs.handler.execute_idle_move();
            THEN("error is noted but not handled") {
                REQUIRE(!test_objs.handler.error_handled);
            }
            WHEN(
                "idle move is executed the second time and the error is still "
                "detected") {
                test_objs.handler.execute_idle_move();
                THEN("error is handled") {
                    REQUIRE(test_objs.handler.error_handled);
                }
            }
            WHEN(
                "idle move is executed the second time and but the error is no "
                "longer detected") {
                test_objs.handler.set_enc_idle_state(true);
                test_objs.hw.set_encoder_value(0);
                test_objs.handler.execute_idle_move();
                THEN("error is not handled") {
                    REQUIRE(!test_objs.handler.error_handled);
                }
            }
        }
    }
}

SCENARIO("handler recovers from error state") {
    BrushedMotorContainer test_objs{};
    auto og_motor_state = GENERATE(BrushedMotorState::FORCE_CONTROLLING_HOME,
                                   BrushedMotorState::FORCE_CONTROLLING,
                                   BrushedMotorState::POSITION_CONTROLLING);
    auto stay_engaged = GENERATE(true, false);

    GIVEN("an error occurred during an idle move") {
        test_objs.hw.set_motor_state(og_motor_state);
        test_objs.hw.set_stay_enabled(stay_engaged);
        test_objs.handler.error_handled = true;

        WHEN("a cancel request is received") {
            test_objs.hw.set_cancel_request(
                can::ids::ErrorSeverity::warning,
                can::ids::ErrorCode::stop_requested);
            test_objs.handler.run_interrupt();
            THEN(
                "motor state should become un-homed only if stay engaged is "
                "falsy") {
                REQUIRE(test_objs.hw.get_motor_state() ==
                        (!stay_engaged ? BrushedMotorState::UNHOMED
                                       : og_motor_state));
            }
            THEN("a stop requested warning is issued") {
                // TODO: do we need to increase the error count if it's only
                // warning about a stop request?
                REQUIRE(test_objs.reporter.messages.size() == 2);
                can::messages::ErrorMessage err =
                    std::get<can::messages::ErrorMessage>(
                        test_objs.reporter.messages.front());
                REQUIRE(err.error_code == can::ids::ErrorCode::stop_requested);
                REQUIRE(err.severity == can::ids::ErrorSeverity::warning);
            }
            THEN("error handled is not cleared, yet") {
                REQUIRE(test_objs.handler.error_handled);
            }
            AND_WHEN("a new message is loaded and executed") {
                test_objs.queue.try_write_isr(grip_msg);
                test_objs.handler.update_and_start_move();
                THEN(
                    "error state is cleared and the new move is being "
                    "executed") {
                    REQUIRE(!test_objs.handler.error_handled);
                    REQUIRE(test_objs.hw.get_motor_state() ==
                            BrushedMotorState::FORCE_CONTROLLING);
                    REQUIRE(test_objs.handler.has_active_move());
                    REQUIRE(test_objs.hw.get_stay_enabled());
                }
            }
        }
    }
}