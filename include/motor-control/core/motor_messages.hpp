#pragma once

#include <cstdint>

#include "can/core/messages.hpp"
#include "motor-control/core/types.hpp"

namespace motor_messages {

using um_per_tick = can_messages::um_per_tick;
using um_per_tick_sq = can_messages::um_per_tick_sq;

struct MotionConstraints {
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;
};

enum class MoveStopCondition : uint8_t {
    none = 0x0,
    limit_switch = 0x1,
    cap_sensor = 0x2
};

struct Move {        // NOLINT(cppcoreguidelines-pro-type-member-init)
    ticks duration;  // in ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
    MoveStopCondition stop_condition = MoveStopCondition::none;
};

struct BrushedMove {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    ticks duration;   // in ticks
    uint32_t freq;
    uint32_t duty_cycle;
    uint8_t group_id;
    uint8_t seq_id;
    MoveStopCondition stop_condition = MoveStopCondition::none;
};

const uint8_t NO_GROUP = 0xff;

enum class AckMessageId : uint8_t {
    complete_without_condition = 0x1,
    stopped_by_condition = 0x2,
    timeout = 0x3,
    position_error = 0x4
};

struct Ack {
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_steps;
    uint32_t encoder_position;
    AckMessageId ack_id;
};

constexpr const int RADIX = 31;

}  // namespace motor_messages
