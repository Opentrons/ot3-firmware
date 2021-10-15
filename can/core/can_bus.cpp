#include "can/core/can_bus.hpp"

using namespace can_bus;

/**
 * Round a length to a CAN FD length.
 *
 * @param length a message payload length
 * @return a CanFDMessageLength
 */
auto can_bus::to_canfd_length(uint32_t length) -> CanFDMessageLength {
    switch (length) {
        case 0:
            return CanFDMessageLength::l0;
        case 1:
            return CanFDMessageLength::l1;
        case 2:
            return CanFDMessageLength::l2;
        case 3:
            return CanFDMessageLength::l3;
        case 4:
            return CanFDMessageLength::l4;
        case 5:
            return CanFDMessageLength::l5;
        case 6:
            return CanFDMessageLength::l6;
        case 7:
            return CanFDMessageLength::l7;
        case 8:
            return CanFDMessageLength::l8;
    }
    if (length <= 12)
        return CanFDMessageLength::l12;
    else if (length <= 16)
        return CanFDMessageLength::l16;
    else if (length <= 20)
        return CanFDMessageLength::l20;
    else if (length <= 24)
        return CanFDMessageLength::l24;
    else if (length <= 32)
        return CanFDMessageLength::l32;
    else if (length <= 48)
        return CanFDMessageLength::l48;
    else
        return CanFDMessageLength::l64;
}

/**
 * Set up the can bus receive filters.
 *
 * @param can_filters CanBusFilters interface.
 * @param node_id The node id to allow.
 */
void can_bus::setup_node_id_filter(CanBusFilters& can_filters, NodeId node_id) {
    // The node id mask.
    auto node_id_mask = can_arbitration_id::ArbitrationId{
        .parts = {.function_code = 0, .node_id = 0xFF, .message_id = 0}};

    // Set up the broadcast filter
    auto filter = can_arbitration_id::ArbitrationId{.id = 0};
    filter.parts.node_id = static_cast<uint32_t>(NodeId::broadcast);
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0,
                           filter.id, node_id_mask.id);

    // Set up the specific node_id filter
    filter.id = 0;
    filter.parts.node_id = static_cast<uint32_t>(node_id);
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1,
                           filter.id, node_id_mask.id);

    // Reject everything else
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);
}
