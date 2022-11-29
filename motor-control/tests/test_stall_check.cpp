#include "catch2/catch.hpp"
#include "motor-control/core/stall_check.hpp"

TEST_CASE("stall itr check functionality") {
    GIVEN("1 step per µm for encoder and stepper, 10 µm stall threshold") {
        auto subject = stall_check::StallCheck(1, 1, 10);
        WHEN("stepper count is zero") {
            subject.reset_itr_counts(0);
            THEN("acceptable encoder positions are not stalls") {
                int32_t encoder_ticks = GENERATE(0, -10, 10);
                REQUIRE(subject.check_stall_itr(encoder_ticks));
            }
            THEN("unacceptable encoder positions are stalls") {
                int32_t encoder_ticks = GENERATE(11, -11);
                REQUIRE(!subject.check_stall_itr(encoder_ticks));
            }
        }
        WHEN("stepper count is set to 123") {
            subject.reset_itr_counts(123);
            THEN("acceptable encoder positions are not stalls") {
                int32_t encoder_ticks = GENERATE(123, 113, 133);
                REQUIRE(subject.check_stall_itr(encoder_ticks));
            }
            THEN("unacceptable encoder positions are stalls") {
                int32_t encoder_ticks = GENERATE(112, 134);
                REQUIRE(!subject.check_stall_itr(encoder_ticks));
            }
        }
    }
    GIVEN("10 step/µm stepper, 1 step/µm encoder, 10 µm stall threshold") {
        auto subject = stall_check::StallCheck(1, 10, 10);
        WHEN("stepper count is set to 123") {
            subject.reset_itr_counts(123);
            THEN("acceptable encoder positions are not stalls") {
                int32_t encoder_ticks = GENERATE(12, 3, 21);
                REQUIRE(subject.check_stall_itr(encoder_ticks));
            }
            THEN("unacceptable encoder positions are stalls") {
                int32_t encoder_ticks = GENERATE(1, 23);
                REQUIRE(!subject.check_stall_itr(encoder_ticks));
            }
        }
        WHEN("stepper count is set to 1230") {
            subject.reset_itr_counts(1230);
            THEN("acceptable encoder positions are not stalls") {
                int32_t encoder_ticks = GENERATE(123, 113, 133);
                REQUIRE(subject.check_stall_itr(encoder_ticks));
            }
            THEN("unacceptable encoder positions are stalls") {
                int32_t encoder_ticks = GENERATE(112, 134);
                REQUIRE(!subject.check_stall_itr(encoder_ticks));
            }
        }
    }
    GIVEN("1 step/µm stepper, 10 step/µm encoder, 10 µm stall threshold") {
        auto subject = stall_check::StallCheck(10, 1, 10);
        WHEN("stepper count is set to 123") {
            subject.reset_itr_counts(123);
            THEN("acceptable encoder positions are not stalls") {
                int32_t encoder_ticks = GENERATE(1230, 1130, 1330);
                REQUIRE(subject.check_stall_itr(encoder_ticks));
            }
            THEN("unacceptable encoder positions are stalls") {
                int32_t encoder_ticks = GENERATE(1129, 1331);
                REQUIRE(!subject.check_stall_itr(encoder_ticks));
            }
        }
    }
}

TEST_CASE("stall check isr step handling") {
    float stepper_begin_um = GENERATE(0.0, 100.0, 10000.0, 123456.0);
    float stepper_tick_per_um = GENERATE(50, 100, 200);
    float encoder_tick_per_um = 100;
    float stall_threshold_um = 50;
    int32_t stepper_begin_steps =
        static_cast<int32_t>(stepper_begin_um * stepper_tick_per_um);
    GIVEN("a stall check initialized to " << stepper_begin_um
                                          << " micrometers") {
        auto subject = stall_check::StallCheck(
            encoder_tick_per_um, stepper_tick_per_um, stall_threshold_um);
        subject.reset_itr_counts(stepper_begin_steps);
        WHEN("stepping the motor less than the stall threshold") {
            bool got_a_true = false;
            for (int i = 0; i < (stall_threshold_um * stepper_tick_per_um) - 1;
                 ++i) {
                if (subject.step_itr(true)) {
                    got_a_true = true;
                }
            }
            THEN("no stallcheck was flagged") { REQUIRE(!got_a_true); }
        }
        WHEN("stepping the motor positively until the stall threshold") {
            int steps = 0;
            int expected =
                static_cast<int>(stepper_tick_per_um * stall_threshold_um);
            while (steps < (expected * 2)) {
                if (subject.step_itr(true)) {
                    break;
                }
                steps++;
            }
            REQUIRE(steps == expected);
            AND_WHEN("checking stall status") {
                float encoder_position = stepper_begin_um + stall_threshold_um;
                THEN("valid encoder position returns true: "
                     << encoder_position) {
                    REQUIRE(subject.check_stall_itr(static_cast<int32_t>(
                        encoder_position * encoder_tick_per_um)));
                }
                THEN("invalid encoder position returns false") {
                    encoder_position += stall_threshold_um + 10;
                    REQUIRE(!subject.check_stall_itr(static_cast<int32_t>(
                        encoder_position * encoder_tick_per_um)));
                }
            }
        }
        WHEN("stepping the motor negatively until the stall threshold") {
            int steps = 0;
            int expected =
                static_cast<int>(stepper_tick_per_um * stall_threshold_um);
            while (steps < (expected * 2)) {
                if (subject.step_itr(false)) {
                    break;
                }
                steps++;
            }
            REQUIRE(steps == expected);
            AND_WHEN("checking stall status") {
                float encoder_position = stepper_begin_um - stall_threshold_um;
                THEN("valid encoder position returns true: "
                     << encoder_position) {
                    REQUIRE(subject.check_stall_itr(static_cast<int32_t>(
                        encoder_position * encoder_tick_per_um)));
                }
                THEN("invalid encoder position returns false") {
                    encoder_position -= stall_threshold_um + 10;
                    REQUIRE(!subject.check_stall_itr(static_cast<int32_t>(
                        encoder_position * encoder_tick_per_um)));
                }
            }
        }
    }
}

TEST_CASE("stall check handling with no encoder") {
    GIVEN("a stall check with no encoder") {
        auto subject = stall_check::StallCheck(0, 1, 10);
        THEN("the step function always returns false") {
            bool got_a_true = false;
            for (int i = 0; i < 100; ++i) {
                if (subject.step_itr(true)) {
                    got_a_true = true;
                }
                if (subject.step_itr(false)) {
                    got_a_true = true;
                }
            }
            REQUIRE(!got_a_true);
        }
        THEN("the interrupt stall check always returns true") {
            int32_t encoder = GENERATE(0, 100, 10000, 1000000);
            REQUIRE(subject.check_stall_itr(encoder));
        }
    }
}
