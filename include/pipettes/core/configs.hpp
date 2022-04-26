#pragma once

#include "motor-control/core/linear_motion_system.hpp"
#include "pipettes/core/pipette_type.h"

namespace configs {

auto linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig>;

}  // namespace configs