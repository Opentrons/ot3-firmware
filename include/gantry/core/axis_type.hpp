#pragma once
#include "can/core/ids.hpp"

/**
 * Methods that are generated for specific gantry axis.
 */
namespace axis_type {

/**
 * Get the node id for this gantry's axis type
 *
 * @return A node id
 */
auto get_node_id() -> can_ids::NodeId;

}  // namespace axis_type
