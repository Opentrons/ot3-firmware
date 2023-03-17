#pragma once

#include "motor-control/core/linear_motion_system.hpp"

namespace error_tolerance_config {

// upon advice from hardware, 0.01mm is a good limit for precision
static constexpr double ACCEPTABLE_DISTANCE_TOLERANCE_MM = 2;
static constexpr double UNWANTED_MOVEMENT_DISTANCE_MM = 2;

class BrushedMotorErrorTolerance {
  public:
    BrushedMotorErrorTolerance(
        lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gearbox_config)
        : gear_conf(gearbox_config) {
        acceptable_position_error =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    ACCEPTABLE_DISTANCE_TOLERANCE_MM);
        unwanted_movement_threshold =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    UNWANTED_MOVEMENT_DISTANCE_MM);
    }

    void update_tolerance(double pos_error, double unwanted_movement) {
        acceptable_position_error =
            int32_t(gear_conf.get_encoder_pulses_per_mm() * pos_error);
        unwanted_movement_threshold =
            int32_t(gear_conf.get_encoder_pulses_per_mm() * unwanted_movement);
    }
    lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gear_conf;
    int32_t acceptable_position_error = 0;
    int32_t unwanted_movement_threshold = 0;
};

}  // namespace error_tolerance_config
