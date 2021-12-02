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

/**
 * Message id mask.
 */
constexpr auto message_id_mask = ArbitrationId{.parts{
    .function_code = 0, .node_id = 0, .message_id = 0x3FFF, .padding = 0}};

/**
 * Node id mask.
 */
constexpr auto node_id_mask = ArbitrationId{
    .parts{.function_code = 0, .node_id = 0xFF, .message_id = 0, .padding = 0}};

}  // namespace can_arbitration_id