#pragma once

#include <cstdint>

typedef int32_t sq0_31;  // 0: signed bit,  1-31: fractional bits
typedef uint64_t
    q31_31;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

namespace motor_messages {

struct GenericMove {
    uint32_t duration;
    sq0_31 acceleration;
    sq0_31 velocity;
};

struct MoveGroupMove {
    uint32_t duration;  // in ticks
    sq0_31 velocity;
    sq0_31 acceleration;
    uint8_t group_id;
    uint8_t seq_id;
};

template <typename MT>
concept MoveType = requires {
    std::is_same_v<MT, GenericMove> || std::is_same_v<MT, MoveGroupMove>;
};

enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Ack {
    uint8_t group_id;
    uint8_t seq_id;
    AckMessageId ack_id;
};

}  // namespace motor_messages
