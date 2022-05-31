#pragma once

#include <cstdint>

#include "can/core/ids.hpp"
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

struct GearMotorAck : public Ack {
    can_ids::PipetteTipActionType action;
};

struct Move {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    stepper_timer_ticks duration;  // in stepper timer ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
    MoveStopCondition stop_condition = MoveStopCondition::none;

    auto build_ack(uint32_t position, uint32_t pulses, AckMessageId _id)
        -> Ack {
        return Ack{
            .group_id = group_id,
            .seq_id = seq_id,
            .current_position_steps = position,
            .encoder_position = pulses,
            .ack_id = _id,
        };
    }
};

struct GearMotorMove : public Move {
    can_ids::PipetteTipActionType action;

    auto build_ack(uint32_t position, uint32_t pulses, AckMessageId _id)
        -> GearMotorAck {
        return GearMotorAck{group_id, seq_id, position, pulses, _id, action};
    }
};

struct BrushedMove {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    /**
     * Note that brushed timer tick at a different frequency from the stepper
     * motor timer.
     */
    brushed_timer_ticks duration;  // in brushed timer ticks
    uint32_t freq;
    uint32_t duty_cycle;
    uint8_t group_id;
    uint8_t seq_id;
    MoveStopCondition stop_condition = MoveStopCondition::none;
};

const uint8_t NO_GROUP = 0xff;

constexpr const int RADIX = 31;

}  // namespace motor_messages
