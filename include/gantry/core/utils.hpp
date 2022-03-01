#pragma once

#include "can/core/ids.hpp"
#include "gantry/core/axis_type.h"
#include "motor-control/core/tmc2130_registers.hpp"

namespace utils {

auto get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId;

auto get_node_id() -> can_ids::NodeId;

auto register_config_by_axis(enum GantryAxisType which)
    -> tmc2130::TMC2130RegisterMap;

auto register_config() -> tmc2130::TMC2130RegisterMap;

}  // namespace utils