#include "can/core/can_bus.hpp"

#include "can/core/types.h"

using namespace can::bus;

/**
 * Set up the can bus receive filters.
 *
 * @param can_filters CanBusFilters interface.
 * @param node_id The node id to allow.
 */
void can::bus::CanBus::setup_node_id_filter(NodeId node_id) {
    // Set up the broadcast filter
    auto filter = can::arbitration_id::ArbitrationId();
    filter.node_id(NodeId::broadcast);
    add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0, filter,
               can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Set up the specific node_id filter
    filter.node_id(node_id);
    add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
               can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Reject everything else
    add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);
}
