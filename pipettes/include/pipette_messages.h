#pragma once

#include <variant>


namespace pipette_messages {

    enum class MessageType {
        stop = 0x00,
        set_speed = 0x01,
        get_speed = 0x10,
        get_speed_result = 0x11
    };

    struct Empty {
    };

    struct Speed {
        uint32_t mm_sec;
    };

    using PayloadType = std::variant<Speed, Empty>;

    using Message = std::pair<MessageType, PayloadType>;
}
