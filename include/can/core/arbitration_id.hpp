#pragma once

#include <cstdint>

namespace can_arbitration_id {

/**
 * The components of a 29-bit arbitration id as a bitfield.
 */
struct ArbitrationIdParts {
    unsigned function_code : 7;
    unsigned node_id : 8;
    unsigned message_id : 14;
    unsigned int padding : 3;
};

/**
 * A union of arbitration id in parts or as an integer.
 */
union ArbitrationId {
    ArbitrationIdParts parts;
    uint32_t id;
};

}