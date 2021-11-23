#pragma once

#include "gantry/core/axis_type.h"
#include "can/core/ids.hpp"
#include "motor-control/core/motor_driver_config.hpp"

namespace utils {

auto get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId;

auto get_node_id() -> can_ids::NodeId;

auto register_config_by_axis(enum GantryAxisType which) -> motor_driver_config::RegisterConfig;

auto register_config() -> motor_driver_config::RegisterConfig;

}