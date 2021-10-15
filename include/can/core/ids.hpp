#pragma once

#include <cstdint>

namespace can_ids {

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

/**
 * Node id definitions.
 */
enum class NodeId : uint8_t {
    broadcast = 0x00,
    host = 0x10,
    pipette = 0x20,
    gantry_x = 0x30,
    gantry_y = 0x40,
    head = 0x50,
};

/**
 * Message Id definitions.
 */
enum class MessageId : uint16_t {
    heartbeat_request = 0x3FFF,
    heartbeat_response = 0x3FFE,

    device_info_request = 0x3002,
    device_info_response = 0x3003,

    stop_request = 0x00,

    get_status_request = 0x01,
    get_status_response = 0x05,

    enable_motor_request = 0x06,
    disable_motor_request = 0x07,

    move_request = 0x10,

    setup_request = 0x02,

    get_speed_request = 0x04,
    get_speed_response = 0x11,

    write_eeprom = 0x2001,
    read_eeprom_request = 0x2002,
    read_eeprom_response = 0x2003
};

}  // namespace can_ids