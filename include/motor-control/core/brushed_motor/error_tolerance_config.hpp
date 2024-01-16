#pragma once

#include "motor-control/core/linear_motion_system.hpp"

namespace error_tolerance_config {

// upon advice from hardware, 0.01mm is a good limit for precision
static constexpr double ACCEPTABLE_DISTANCE_TOLERANCE_MM = 2;
static constexpr double UNWANTED_MOVEMENT_DISTANCE_MM = 2;

// hold off for 1 ms (with a 32k Hz timer)
// using the logic analyzer it takes about 0.2-0.3 ms for the output
// to stablize after changing directions of the PWM
static constexpr double IDLE_HOLDOFF_MS = 1;

class BrushedMotorErrorTolerance {
  public:
    BrushedMotorErrorTolerance(
        lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gearbox_config,
        double pwm_timer_freq)
        : gear_conf(gearbox_config), timer_freq(pwm_timer_freq) {
        acceptable_position_error =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    ACCEPTABLE_DISTANCE_TOLERANCE_MM);
        unwanted_movement_threshold =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    UNWANTED_MOVEMENT_DISTANCE_MM);
        idle_holdoff_ticks = uint32_t(timer_freq / 1000 * IDLE_HOLDOFF_MS);
    }

    void update_tolerance(double pos_error, double unwanted_movement) {
        acceptable_position_error =
            int32_t(gear_conf.get_encoder_pulses_per_mm() * pos_error);
        unwanted_movement_threshold =
            int32_t(gear_conf.get_encoder_pulses_per_mm() * unwanted_movement);
    }

    void update_idle_holdoff_ticks(double holdoff_ms) {
        idle_holdoff_ticks = uint32_t(timer_freq / 1000 * holdoff_ms);
    }

    [[nodiscard]] auto get_holdoff_ms() const -> double {
        return double(idle_holdoff_ticks * 1000 / timer_freq);
    }

    lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gear_conf;
    double timer_freq;
    int32_t acceptable_position_error = 0;
    int32_t unwanted_movement_threshold = 0;
    uint32_t idle_holdoff_ticks = 0;
};

}  // namespace error_tolerance_config
