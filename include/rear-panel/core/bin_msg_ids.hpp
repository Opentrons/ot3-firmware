#pragma once
#include <cstdint>
namespace messages {
// TODO(ryan): as part of RET-1307 create an auto gen of the ids like the can
// messages
enum class MessageType : uint16_t {
    ECHO = 0x00,
    DEVICE_INFO_REQ = 0x01,
    DEVICE_INFO_RESP = 0x02,
};

};  // namespace messages
