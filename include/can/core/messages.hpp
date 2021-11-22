#pragma once

#include <cstdint>
#include <span>

#include "common/core/bit_utils.hpp"
#include "ids.hpp"
#include "parse.hpp"

namespace can_messages {

using namespace can_ids;

using ticks = uint32_t;
using um_per_tick = int32_t;
using um_per_tick_sq = int32_t;

/**
 * These types model the messages being sent and received over the can bus.
 *
 * The objects must implement the Parsable concept to deserialize the payload.
 */

template <MessageId MId>
struct BaseMessage {
    /** Satisfy the HasMessageID concept */
    static const auto id = MId;
    bool operator==(const BaseMessage& other) const = default;
};

/**
 * A message with no payload.
 *
 * @tparam MId
 */
template <MessageId MId>
struct Empty : BaseMessage<MId> {
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> Empty {
        return Empty{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }

    bool operator==(const Empty& other) const = default;
};

template <MessageId MId>
struct Response : BaseMessage<MId> {
    NodeId node_id;

    void set_node_id(NodeId id) { node_id = id; }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize_node_id(Output body, Limit limit) const {
        return bit_utils::int_to_bytes(static_cast<uint8_t>(node_id), body,
                                       limit);
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        return iter - body;
    }
};

using HeartbeatRequest = Empty<MessageId::heartbeat_request>;

using HeartbeatResponse = Empty<MessageId::heartbeat_response>;

using DeviceInfoRequest = Empty<MessageId::device_info_request>;

struct DeviceInfoResponse : Response<MessageId::device_info_response> {
    /**
     *   TODO (al, 2021-09-13)
     *   Seth's thoughts on future of payload
     *   IMO we should set this up for a couple more things than just version,
     * and then can care about version in general a little less. I don't think
     * it's necessarily critical to get right the first time, but we could do
     * something like:
     *   - two bits of build type (dev, testing, release)
     *   - a byte or two of like message schema version
     *   - a four byte incrementing version seems fine i suppose
     *   - a byte or two for well-known-id system identification through some
     * centrally defined enum
     *   - at some point we'll want a serial number probably
     *   - a hardware revision unless we want to fold that into the
     * well-known-id
     */
    uint32_t version;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        return iter - body;
    }
    bool operator==(const DeviceInfoResponse& other) const = default;
};

using StopRequest = Empty<MessageId::stop_request>;

using GetStatusRequest = Empty<MessageId::get_status_request>;

using EnableMotorRequest = Empty<MessageId::enable_motor_request>;

using DisableMotorRequest = Empty<MessageId::disable_motor_request>;

struct GetStatusResponse : Response<MessageId::get_status_response> {
    uint8_t status;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(status, iter, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }
    bool operator==(const GetStatusResponse& other) const = default;
};

struct MoveRequest : BaseMessage<MessageId::move_request> {
    ticks duration;
    um_per_tick velocity;
    um_per_tick_sq acceleration;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MoveRequest {
        ticks duration = 0;
        um_per_tick velocity = 0;
        um_per_tick_sq acceleration = 0;
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        return MoveRequest{.duration = duration,
                           .velocity = velocity,
                           .acceleration = acceleration};
    }

    bool operator==(const MoveRequest& other) const = default;
};

using SetupRequest = Empty<MessageId::setup_request>;

struct WriteToEEPromRequest : BaseMessage<MessageId::write_eeprom> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToEEPromRequest {
        uint8_t serial_number = 0;
        body = bit_utils::bytes_to_int(body, limit, serial_number);
        return WriteToEEPromRequest{.serial_number = serial_number};
    }

    bool operator==(const WriteToEEPromRequest& other) const = default;
};

using ReadFromEEPromRequest = Empty<MessageId::read_eeprom_request>;

struct ReadFromEEPromResponse : Response<MessageId::read_eeprom_response> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(serial_number, iter, limit);
        return iter - body;
    }
    bool operator==(const ReadFromEEPromResponse& other) const = default;
};

struct AddLinearMoveRequest : BaseMessage<MessageId::add_linear_move_request> {
    uint8_t group_id;
    uint8_t seq_id;
    ticks duration;
    um_per_tick_sq acceleration;
    um_per_tick velocity;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddLinearMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        ticks duration = 0;
        um_per_tick_sq acceleration = 0;
        um_per_tick velocity = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        return AddLinearMoveRequest{.group_id = group_id,
                                    .seq_id = seq_id,
                                    .duration = duration,
                                    .acceleration = acceleration,
                                    .velocity = velocity};
    }

    bool operator==(const AddLinearMoveRequest& other) const = default;
};

struct GetMoveGroupRequest : BaseMessage<MessageId::get_move_group_request> {
    uint8_t group_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetMoveGroupRequest {
        uint8_t group_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        return GetMoveGroupRequest{.group_id = group_id};
    }

    bool operator==(const GetMoveGroupRequest& other) const = default;
};

struct GetMoveGroupResponse : Response<MessageId::get_move_group_response> {
    uint8_t group_id;
    uint8_t num_moves;
    uint32_t total_duration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(group_id, iter, limit);
        iter = bit_utils::int_to_bytes(num_moves, iter, limit);
        iter = bit_utils::int_to_bytes(total_duration, iter, limit);
        return iter - body;
    }
    bool operator==(const GetMoveGroupResponse& other) const = default;
};

struct ExecuteMoveGroupRequest
    : BaseMessage<MessageId::execute_move_group_request> {
    uint8_t group_id;
    uint8_t start_trigger;
    uint8_t cancel_trigger;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ExecuteMoveGroupRequest {
        uint8_t group_id = 0;
        uint8_t start_trigger = 0;
        uint8_t cancel_trigger = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, start_trigger);
        body = bit_utils::bytes_to_int(body, limit, cancel_trigger);
        return ExecuteMoveGroupRequest{.group_id = group_id,
                                       .start_trigger = start_trigger,
                                       .cancel_trigger = cancel_trigger};
    }

    bool operator==(const ExecuteMoveGroupRequest& other) const = default;
};

using ClearAllMoveGroupsRequest =
    Empty<MessageId::clear_all_move_groups_request>;

struct MoveCompleted : Response<MessageId::move_completed> {
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position;
    uint8_t ack_id;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(group_id, iter, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(current_position, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        return iter - body;
    }

    bool operator==(const MoveCompleted& other) const = default;
};

struct SetMotionConstraints : BaseMessage<MessageId::set_motion_constraints> {
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetMotionConstraints {
        um_per_tick min_velocity = 0;
        um_per_tick max_velocity = 0;
        um_per_tick_sq min_acceleration = 0;
        um_per_tick_sq max_acceleration = 0;
        body = bit_utils::bytes_to_int(body, limit, min_velocity);
        body = bit_utils::bytes_to_int(body, limit, max_velocity);
        body = bit_utils::bytes_to_int(body, limit, min_acceleration);
        body = bit_utils::bytes_to_int(body, limit, max_acceleration);
        return SetMotionConstraints{.min_velocity = min_velocity,
                                    .max_velocity = max_velocity,
                                    .min_acceleration = min_acceleration,
                                    .max_acceleration = max_acceleration};
    }

    bool operator==(const SetMotionConstraints& other) const = default;
};

using GetMotionConstraintsRequest =
    Empty<MessageId::get_motion_constraints_request>;

struct GetMotionConstraintsResponse
    : Response<MessageId::set_motion_constraints> {
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(min_velocity, iter, limit);
        iter = bit_utils::int_to_bytes(max_velocity, iter, limit);
        iter = bit_utils::int_to_bytes(min_acceleration, iter, limit);
        iter = bit_utils::int_to_bytes(max_acceleration, iter, limit);
        return iter - body;
    }

    bool operator==(const GetMotionConstraintsResponse& other) const = default;
};

struct WriteMotorDriverRegister
    : BaseMessage<MessageId::write_motor_driver_register_request> {
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t data = 0;
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        body = bit_utils::bytes_to_int(body, limit, data);
        return WriteMotorDriverRegister{.reg_address = reg_address,
                                        .data = data};
    }

    bool operator==(const WriteMotorDriverRegister& other) const = default;
};

struct ReadMotorDriverRegister
    : BaseMessage<MessageId::read_motor_driver_register_request> {
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadMotorDriverRegister {
        uint8_t reg_address = 0;
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return ReadMotorDriverRegister{.reg_address = reg_address};
    }

    bool operator==(const ReadMotorDriverRegister& other) const = default;
};

struct ReadMotorDriverRegisterResponse
    : Response<MessageId::read_motor_driver_register_response> {
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = serialize_node_id(body, limit);
        iter = bit_utils::int_to_bytes(reg_address, iter, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }

    bool operator==(const ReadMotorDriverRegisterResponse& other) const =
        default;
};

}  // namespace can_messages
