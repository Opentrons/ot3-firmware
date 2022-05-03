#pragma once

#include "can/core/ids.hpp"
#include "gantry/core/axis_type.h"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"

namespace utils {

auto get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId;

auto get_node_id() -> can_ids::NodeId;

auto linear_motion_sys_config_by_axis(enum GantryAxisType which)
    -> lms::LinearMotionSystemConfig<lms::BeltConfig>;

auto linear_motion_system_config()
    -> lms::LinearMotionSystemConfig<lms::BeltConfig>;
}  // namespace utils
