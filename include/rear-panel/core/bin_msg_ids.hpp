#pragma once
#include <cstdint>
namespace rearpanel {
namespace messages {
// TODO(ryan): as part of RET-1307 create an auto gen of the ids like the can
// messages
enum class MessageType : uint16_t {
    ECHO = 0x00,
    ACK = 0x01,
    ACK_FAILED = 0x02,
    DEVICE_INFO_REQ = 0x03,
    DEVICE_INFO_RESP = 0x04,
    ENTER_BOOTLOADER = 0x05,
    ENTER_BOOTLOADER_RESPONSE = 0x06,
    ENGAGE_ESTOP_REQUEST = 0x07,
    RELEASE_ESTOP_REQUEST = 0x08,
    ENGAGE_SYNC_REQUEST = 0x09,
    RELEASE_SYNC_REQUEST = 0x0A,
    ESTOP_STATE_CHANGE = 0x0B,
    ESTOP_BUTTON_DETECTION_CHANGE = 0x0C,
    DOOR_SWITCH_STATE_REQUEST = 0x0D,
    DOOR_SWITCH_STATE_INFO = 0x0E,
};

};  // namespace messages
};  // namespace rearpanel
