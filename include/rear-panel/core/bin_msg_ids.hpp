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
    ENABLE_ESTOP_REQUEST = 0x07,
    DISABLE_ESTOP_REQUEST = 0x08,
    ENABLE_SYNC_REQUEST = 0x09,
    DISABLE_SYNC_REQUEST = 0x0A,
};

};  // namespace messages
};  // namespace rearpanel
