#pragma once

#include "motor-control/core/linear_motion_system.hpp"
#include "pipettes/core/pipette_type.h"

namespace configs {

// Number of full steps the stall threshold should equate to
constexpr float STALL_THRESHOLD_UM = 500;
;

auto linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig>;

auto gear_motion_sys_config()
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig>;

}  // namespace configs