#pragma once

#include "can/core/ids.hpp"
#include "gantry/core/axis_type.h"
#include "motor-control/core/tmc2130_config.hpp"

namespace utils {

auto get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId;

auto get_node_id() -> can_ids::NodeId;

auto driver_config_by_axis(enum GantryAxisType which)
    -> tmc2130::TMC2130DriverConfig;

auto driver_config() -> tmc2130::TMC2130DriverConfig;

}  // namespace utils