#pragma once

#include <variant>

#include "motor-control/core/motor_messages.hpp"

namespace gantry_messages {

enum class MessageType : uint8_t {
    stop = 0x00,
    status = 0x01,
    move = 0x10,
    setup = 0x02,
    set_speed = 0x03,
    get_speed = 0x04,
    get_speed_result = 0x11,
    get_status_result = 0x05
};

struct Stop {};

struct SetSpeed {
    uint32_t mm_sec;
};

struct GetSpeed {};

struct GetSpeedResult {
    uint32_t mm_sec;
};

struct GetStatusResult {
    uint8_t status;
    uint32_t data;
};

struct Status {};

struct Setup {};

using ReceivedMessage = std::variant<std::monostate, Stop, SetSpeed, GetSpeed,
                                     motor_messages::CanMove, Setup, Status>;

using SentMessage =
    std::variant<std::monostate, GetSpeedResult, GetStatusResult>;

}  // namespace gantry_messages
