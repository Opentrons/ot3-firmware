#pragma once
#include <cstdint>
namespace messages {
// TODO(ryan): as part of RET-1307 create an auto gen of the ids like the can
// messages
enum class MessageType : uint16_t {
    ECHO = 0x00,
    ACK = 0x01,
    ACK_FAILED = 0x02,
    DEVICE_INFO_REQ = 0x03,
    DEVICE_INFO_RESP = 0x04,
};

};  // namespace messages
