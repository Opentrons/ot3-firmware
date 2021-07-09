#pragma once

#include <cstdint>

namespace can_ids {

/**
 * The components of a 29-bit arbitration id as a bitfield.
 */
struct ArbitrationIdParts {
    unsigned int function_code: 7;
    unsigned int node_id: 8;
    unsigned int message_id: 14;
    unsigned int padding: 3;
};

/**
 * A union of arbitration id in parts or as an integer.
 */
union ArbitrationId {
    ArbitrationIdParts parts;
    uint32_t id;
};


/**
 * Message Ids
 */
enum class MessageId : uint16_t {
    heartbeat_request,
    heartbeat_response,

    device_info_request,
    device_info_response,
};

}