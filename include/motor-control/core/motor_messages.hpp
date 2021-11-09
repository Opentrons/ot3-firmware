#pragma once

#include <cstdint>

#include "can/core/messages.hpp"

typedef int32_t sq0_31;  // 0: signed bit,  1-31: fractional bits
typedef uint64_t
    q31_31;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using ticks = uint64_t;
using steps_per_tick = sq0_31;
using steps_per_tick_sq = sq0_31;
using steps = q31_31;

namespace motor_messages {

using namespace can_messages;

struct Move {
    ticks duration;  // in ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
};

using MoveStatus = GetMoveStatusResponse;

const uint8_t NO_GROUP = 0xff;

enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Ack {
    uint8_t group_id;
    uint8_t seq_id;
    AckMessageId ack_id;
};

}  // namespace motor_messages
