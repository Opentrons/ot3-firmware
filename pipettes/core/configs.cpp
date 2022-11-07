#include "pipettes/core/configs.hpp"

auto configs::linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig> {
    switch (which) {
        case PipetteType::NINETY_SIX_CHANNEL:
        case PipetteType::THREE_EIGHTY_FOUR_CHANNEL:
            return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
                .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 2},
                .steps_per_rev = 200,
                .microstep = 64,
                .encoder_pulses_per_rev = 1000};
        case PipetteType::EIGHT_CHANNEL:
        case PipetteType::SINGLE_CHANNEL:
        default:
            return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
                .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 3},
                .steps_per_rev = 200,
                .microstep = 32,
                .encoder_pulses_per_rev = 1000};
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
