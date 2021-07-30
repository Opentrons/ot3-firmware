#pragma once

#include <cstdint>

namespace can_ids {

/**
 * The components of a 29-bit arbitration id as a bitfield.
 */
struct ArbitrationIdParts {
    unsigned int function_code : 7;
    unsigned int node_id : 8;
    unsigned int message_id : 14;
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
 * Function code definitions.
 */
enum class FunctionCode : uint8_t {
    network_management = 0x0,
    sync = 0x2,
    error = 0x4,
    command = 0x10,
    status = 0x12,
    parameters = 0x14,
    bootloader = 0x7c,
    heartbeat = 0x7e
};

enum class NodeId : uint8_t {
    host,
    pipette,
    gantry,
};

/**
 * Message Id definitions.
 */
enum class MessageId : uint16_t {
    heartbeat_request = 0xF000,
    heartbeat_response = 0xF001,

    device_info_request = 0xF002,
    device_info_response = 0xF003,

    stop_request = 0x00,

    get_status_request = 0x01,
    get_status_response = 0x05,

    move_request = 0x10,

    setup_request = 0x02,

    set_speed_request = 0x03,

    get_speed_request = 0x04,
    get_speed_response = 0x11,
};

}  // namespace can_ids