#pragma once

#include <cstdint>
#include <span>

#include "common/core/bit_utils.hpp"
#include "ids.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "parse.hpp"

namespace can_messages {

using namespace can_ids;

using ticks = uint64_t;
using um_per_tick = int16_t;
using um_per_tick_sq = int16_t;

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

using HeartbeatRequest = Empty<MessageId::heartbeat_request>;

using HeartbeatResponse = Empty<MessageId::heartbeat_response>;

using DeviceInfoRequest = Empty<MessageId::device_info_request>;

struct DeviceInfoResponse : BaseMessage<MessageId::device_info_response> {
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
    NodeId node_id;
    uint32_t version;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> DeviceInfoResponse {
        uint8_t node_id;
        uint32_t version;
        body = bit_utils::bytes_to_int(body, limit, node_id);
        body = bit_utils::bytes_to_int(body, limit, version);
        return DeviceInfoResponse{.node_id = static_cast<NodeId>(node_id),
                                  .version = version};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(node_id), body, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        return iter - body;
    }
    bool operator==(const DeviceInfoResponse& other) const = default;
};

using StopRequest = Empty<MessageId::stop_request>;

using GetStatusRequest = Empty<MessageId::get_status_request>;

using EnableMotorRequest = Empty<MessageId::enable_motor_request>;

using DisableMotorRequest = Empty<MessageId::disable_motor_request>;

struct GetStatusResponse : BaseMessage<MessageId::get_status_response> {
    uint8_t status;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetStatusResponse {
        uint8_t status = 0;
        uint32_t data = 0;

        body = bit_utils::bytes_to_int(body, limit, status);
        body = bit_utils::bytes_to_int(body, limit, data);

        return GetStatusResponse{.status = status, .data = data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(status, body, limit);
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

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(duration, body, limit);
        iter = bit_utils::int_to_bytes(velocity, iter, limit);
        iter = bit_utils::int_to_bytes(acceleration, iter, limit);
        return iter - body;
    }
    bool operator==(const MoveRequest& other) const = default;
};

using SetupRequest = Empty<MessageId::setup_request>;

using GetSpeedRequest = Empty<MessageId::get_speed_request>;

struct GetSpeedResponse : BaseMessage<MessageId::get_speed_response> {
    uint32_t mm_sec;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetSpeedResponse {
        uint32_t mm_sec = 0;
        body = bit_utils::bytes_to_int(body, limit, mm_sec);
        return GetSpeedResponse{.mm_sec = mm_sec};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(mm_sec, body, limit);
        return iter - body;
    }
    bool operator==(const GetSpeedResponse& other) const = default;
};

struct WriteToEEPromRequest : BaseMessage<MessageId::write_eeprom> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToEEPromRequest {
        uint8_t serial_number = 0;
        body = bit_utils::bytes_to_int(body, limit, serial_number);
        return WriteToEEPromRequest{.serial_number = serial_number};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(serial_number, body, limit);
        return iter - body;
    }
    bool operator==(const WriteToEEPromRequest& other) const = default;
};

using ReadFromEEPromRequest = Empty<MessageId::read_eeprom_request>;

struct ReadFromEEPromResponse : BaseMessage<MessageId::read_eeprom_response> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromEEPromResponse {
        uint8_t serial_number = 0;
        body = bit_utils::bytes_to_int(body, limit, serial_number);
        return ReadFromEEPromResponse{.serial_number = serial_number};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(serial_number, body, limit);
        return iter - body;
    }
    bool operator==(const ReadFromEEPromResponse& other) const = default;
};

struct AddLinearMoveRequest : BaseMessage<MessageId::add_linear_move_request> {
    uint8_t group_id;
    uint8_t seq_id;
    uint16_t duration;
    int16_t acceleration;
    int16_t velocity;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddLinearMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        uint16_t duration = 0;
        int16_t acceleration = 0;
        int16_t velocity = 0;
        //        uint32_t position = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        //        body = bit_utils::bytes_to_int(body, limit, position);
        return AddLinearMoveRequest{.group_id = group_id,
                                    .seq_id = seq_id,
                                    .duration = duration,
                                    .acceleration = acceleration,
                                    .velocity = velocity};
        //                                    .position = position};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(duration, iter, limit);
        iter = bit_utils::int_to_bytes(acceleration, iter, limit);
        iter = bit_utils::int_to_bytes(velocity, iter, limit);
        //        iter = bit_utils::int_to_bytes(position, iter, limit);
        return iter - body;
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

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        return iter - body;
    }

    bool operator==(const GetMoveGroupRequest& other) const = default;
};

struct GetMoveGroupResponse : BaseMessage<MessageId::get_move_group_response> {
    uint8_t group_id;
    uint8_t num_moves;
    uint32_t total_duration;
    uint8_t node_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetMoveGroupResponse {
        uint8_t group_id = 0;
        uint8_t num_moves = 0;
        uint32_t total_duration = 0;
        uint8_t node_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, num_moves);
        body = bit_utils::bytes_to_int(body, limit, total_duration);
        body = bit_utils::bytes_to_int(body, limit, node_id);
        return GetMoveGroupResponse{.group_id = group_id,
                                    .num_moves = num_moves,
                                    .total_duration = total_duration,
                                    .node_id = node_id};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(num_moves, iter, limit);
        iter = bit_utils::int_to_bytes(total_duration, iter, limit);
        iter = bit_utils::int_to_bytes(node_id, iter, limit);
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

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(start_trigger, iter, limit);
        iter = bit_utils::int_to_bytes(cancel_trigger, iter, limit);
        return iter - body;
    }
    bool operator==(const ExecuteMoveGroupRequest& other) const = default;
};

struct ClearMoveGroupRequest
    : BaseMessage<MessageId::clear_move_group_request> {
    uint8_t group_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ClearMoveGroupRequest {
        uint8_t group_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        return ClearMoveGroupRequest{.group_id = group_id};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        return iter - body;
    }
    bool operator==(const ClearMoveGroupRequest& other) const = default;
};

struct MoveGroupCompleted : BaseMessage<MessageId::move_group_completed> {
    uint8_t group_id;
    uint8_t node_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MoveGroupCompleted {
        uint8_t group_id = 0;
        uint8_t node_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, node_id);
        return MoveGroupCompleted{.group_id = group_id, .node_id = node_id};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(node_id, iter, limit);
        return iter - body;
    }

    bool operator==(const MoveGroupCompleted& other) const = default;
};

struct MoveCompleted : BaseMessage<MessageId::move_completed> {
    uint8_t group_id;
    uint8_t seq_id;
    uint8_t ack_id;
    uint8_t node_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MoveCompleted {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        uint8_t ack_id = 0;
        uint8_t node_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, ack_id);
        body = bit_utils::bytes_to_int(body, limit, node_id);
        return MoveCompleted{.group_id = group_id,
                             .seq_id = seq_id,
                             .ack_id = ack_id,
                             .node_id = node_id};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        iter = bit_utils::int_to_bytes(node_id, iter, limit);
        return iter - body;
    }

    bool operator==(const MoveCompleted& other) const = default;
};

}  // namespace can_messages
