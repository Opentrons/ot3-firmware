#pragma once

#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130_config.hpp"
#include "pipettes/core/pipette_type.h"

namespace configs {

auto driver_config_by_axis(PipetteType which) -> tmc2130::TMC2130DriverConfig;

auto linear_motion_sys_config_by_axis(PipetteType which)
    -> lms::LinearMotionSystemConfig<lms::LeadScrewConfig>;

}  // namespace configs