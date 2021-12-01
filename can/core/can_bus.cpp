#include "can/core/can_bus.hpp"

#include "can/core/types.h"

using namespace can_bus;

/**
 * Set up the can bus receive filters.
 *
 * @param can_filters CanBusFilters interface.
 * @param node_id The node id to allow.
 */
void can_bus::CanBus::setup_node_id_filter(NodeId node_id) {
    // Set up the broadcast filter
    auto filter = can_arbitration_id::ArbitrationId{.id = 0};
    filter.parts.node_id = static_cast<uint32_t>(NodeId::broadcast);
    add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0, filter.id,
               can_arbitration_id::node_id_mask.id);

    // Set up the specific node_id filter
    filter.id = 0;
    filter.parts.node_id = static_cast<uint32_t>(node_id);
    add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter.id,
               can_arbitration_id::node_id_mask.id);

    // Reject everything else
    add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);
}
