#pragma once

#include <cstdint>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/types.hpp"

namespace motor_messages {

using mm_per_tick = can::messages::mm_per_tick;
using um_per_tick_sq = can::messages::um_per_tick_sq;

struct MotionConstraints {
    mm_per_tick min_velocity;
    mm_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;
};

using MoveStopCondition = can::ids::MoveStopCondition;

enum class AckMessageId : uint8_t {
    complete_without_condition = 0x1,
    stopped_by_condition = 0x2,
    timeout = 0x3,
    position_error = 0x4,
    condition_met = 0x8
};

struct Ack {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_steps;
    int32_t encoder_position;
    uint8_t position_flags;
    AckMessageId ack_id;
    int32_t start_encoder_position;
    uint16_t usage_key;
};

struct GearMotorAck : public Ack {
    uint32_t start_step_position;
    can::ids::PipetteTipActionType action;
    can::ids::GearMotorId gear_motor_id;
    uint8_t position_flags;
};

struct Move {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    uint32_t message_index;
    stepper_timer_ticks duration;  // in stepper timer ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
    uint8_t stop_condition = static_cast<uint8_t>(MoveStopCondition::none);
    int32_t start_encoder_position;
    uint16_t usage_key;

    auto build_ack(uint32_t position, int32_t pulses, uint8_t flags,
                   AckMessageId _id) -> Ack {
        return Ack{
            .message_index = message_index,
            .group_id = group_id,
            .seq_id = seq_id,
            .current_position_steps = position,
            .encoder_position = pulses,
            .position_flags = flags,
            .ack_id = _id,
            .start_encoder_position = start_encoder_position,
            .usage_key = usage_key,
        };
    }

    [[nodiscard]] auto check_stop_condition(MoveStopCondition cond) const
        -> bool {
        return ((stop_condition & static_cast<uint8_t>(cond)) ==
                static_cast<uint8_t>(cond));
    }
};

struct SensorSyncMove {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    uint32_t message_index;
    stepper_timer_ticks duration;  // in stepper timer ticks
    steps_per_tick velocity;
    steps_per_tick_sq acceleration;
    uint8_t group_id;
    uint8_t seq_id;
    uint8_t stop_condition = static_cast<uint8_t>(MoveStopCondition::none);
    int32_t start_encoder_position;
    uint16_t usage_key;
    can::ids::SensorId sensor_id;
    can::ids::SensorType sensor_type;
    uint8_t binding_flags;

    auto build_ack(uint32_t position, int32_t pulses, uint8_t flags,
                   AckMessageId _id) -> Ack {
        return Ack{
            .message_index = message_index,
            .group_id = group_id,
            .seq_id = seq_id,
            .current_position_steps = position,
            .encoder_position = pulses,
            .position_flags = flags,
            .ack_id = _id,
            .start_encoder_position = start_encoder_position,
            .usage_key = usage_key,
        };
    }

    [[nodiscard]] auto check_stop_condition(MoveStopCondition cond) const
        -> bool {
        return ((stop_condition & static_cast<uint8_t>(cond)) ==
                static_cast<uint8_t>(cond));
    }
};

struct GearMotorMove  // NOLINT(cppcoreguidelines-pro-type-member-init)
    : public Move {
    uint32_t start_step_position;
    can::ids::PipetteTipActionType action;
    can::ids::GearMotorId gear_motor_id;
    can::ids::SensorId sensor_id;
    can::ids::SensorType sensor_type;
    uint8_t binding_flags;
    auto build_ack(uint32_t position, int32_t pulses, uint8_t flags,
                   AckMessageId _id) -> GearMotorAck {
        return GearMotorAck{message_index, group_id,
                            seq_id,        position,
                            pulses,        flags,
                            _id,           start_encoder_position,
                            usage_key,     start_step_position,
                            action,        gear_motor_id};
    }
};

struct BrushedMove {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    /**
     * Note that brushed timer tick at a different frequency from the stepper
     * motor timer.
     */
    uint32_t message_index;
    brushed_timer_ticks duration;  // in brushed timer ticks
    uint32_t duty_cycle;
    uint8_t group_id;
    uint8_t seq_id;
    int32_t encoder_position;
    uint8_t stay_engaged = 0;
    MoveStopCondition stop_condition = MoveStopCondition::none;
    int32_t start_encoder_position;
    uint16_t usage_key;

    auto build_ack(int32_t pulses, uint8_t flags, AckMessageId _id) -> Ack {
        return Ack{
            .message_index = message_index,
            .group_id = group_id,
            .seq_id = seq_id,
            .current_position_steps = 0,
            .encoder_position = pulses,
            .position_flags = flags,
            .ack_id = _id,
            .start_encoder_position = start_encoder_position,
            .usage_key = usage_key,
        };
    }
};

struct UpdatePositionResponse {
    uint32_t message_index;
    uint32_t stepper_position_counts;
    int32_t encoder_pulses;
    uint8_t position_flags;
};

struct GripperJawStateResponse {
    uint32_t message_index;
    uint8_t jaw_state;
};

const uint8_t NO_GROUP = 0xff;

constexpr const int RADIX = 31;

}  // namespace motor_messages
