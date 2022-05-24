#include "pipettes/core/configs.hpp"

auto configs::linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig> {
    switch (which) {
        case PipetteType::ninety_six_channel:
        case PipetteType::three_eighty_four_channel:
            return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
                .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 2},
                .steps_per_rev = 200,
                .microstep = 32};
        case PipetteType::eight_channel:
        case PipetteType::single_channel:
        default:
            return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
                .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 3.03},
                .steps_per_rev = 200,
                .microstep = 32};
    }
}

auto configs::gear_motion_sys_config()
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig> {
    return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 2},
        .steps_per_rev = 200,
        .microstep = 32,
        .gear_ratio = 2};
}