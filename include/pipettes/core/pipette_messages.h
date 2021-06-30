#pragma once

#include <variant>


namespace pipette_messages {

enum class MessageType {
    stop = 0x00,
    set_speed = 0x01,
    get_speed = 0x10,
    get_speed_result = 0x11
};

struct Stop {
};

struct SetSpeed {
    uint32_t mm_sec;
};

struct GetSpeed {
};

struct GetSpeedResult {
    uint32_t mm_sec;
};

using ReceivedMessage = std::variant<std::monostate, Stop, SetSpeed, GetSpeed>;

using SentMessage = std::variant<std::monostate, GetSpeedResult>;

}

