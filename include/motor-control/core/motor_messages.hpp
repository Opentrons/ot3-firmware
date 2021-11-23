#pragma once

#include <cstdint>

#include "can/core/messages.hpp"

using sq0_31 = int32_t;  // 0: signed bit,  1-31: fractional bits
using q31_31 =
    uint64_t;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using ticks = uint64_t;
using steps_per_tick = sq0_31;
using steps_per_tick_sq = sq0_31;

namespace motor_messages {

using um_per_tick = can_messages::um_per_tick;
using um_per_tick_sq = can_messages::um_per_tick_sq;

struct MotionConstraints {
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;
};

struct Move {
    ticks duration;  // in ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
};

const uint8_t NO_GROUP = 0xff;

enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Ack {
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position;
    AckMessageId ack_id;
};

constexpr const int RADIX = 31;

}  // namespace motor_messages
