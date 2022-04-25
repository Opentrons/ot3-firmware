#include "pipettes/core/configs.hpp"

auto configs::linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig> {
    switch (which) {
        default:
            return lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
                .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 3.03},
                .steps_per_rev = 200,
                .microstep = 32};
    }
}