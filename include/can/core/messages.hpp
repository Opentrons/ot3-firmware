#pragma once

#include <cstdint>
#include <span>

#include "common/core/bit_utils.hpp"
#include "ids.hpp"
#include "parse.hpp"

using namespace can_ids;

namespace can_messages {

/**
 * These types model the messages being sent and received over the can bus.
 *
 * The objects must implement the Parsable concept to deserialize the payload.
 */

template <MessageId MId>
struct BaseMessage {
    /** Satisfy the HasMessageID concept */
    static const auto id = MId;
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
        return DeviceInfoResponse{{}, static_cast<NodeId>(node_id), version};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(node_id), body, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        return iter - body;
    }
};

using StopRequest = Empty<MessageId::stop_request>;

using GetStatusRequest = Empty<MessageId::get_status_request>;

struct GetStatusResponse : BaseMessage<MessageId::get_status_response> {
    uint8_t status;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetStatusResponse {
        uint8_t status = 0;
        uint32_t data = 0;

        body = bit_utils::bytes_to_int(body, limit, status);
        body = bit_utils::bytes_to_int(body, limit, data);

        return GetStatusResponse{{}, status, data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(status, body, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }
};

struct MoveRequest : BaseMessage<MessageId::move_request> {
    uint32_t steps;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MoveRequest {
        uint32_t steps = 0;
        body = bit_utils::bytes_to_int(body, limit, steps);
        return MoveRequest{{}, steps};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(steps, body, limit);
        return iter - body;
    }
};

using SetupRequest = Empty<MessageId::setup_request>;

using GetSpeedRequest = Empty<MessageId::get_speed_request>;

struct GetSpeedResponse : BaseMessage<MessageId::get_speed_response> {
    uint32_t mm_sec;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetSpeedResponse {
        uint32_t mm_sec = 0;
        body = bit_utils::bytes_to_int(body, limit, mm_sec);
        return GetSpeedResponse{{}, mm_sec};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(mm_sec, body, limit);
        return iter - body;
    }
};

struct WriteToEEPromRequest : BaseMessage<MessageId::write_eeprom> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToEEPromRequest {
        uint8_t serial_number = 0;
        body = bit_utils::bytes_to_int(body, limit, serial_number);
        return WriteToEEPromRequest{{}, serial_number};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(serial_number, body, limit);
        return iter - body;
    }
};

using ReadFromEEPromRequest = Empty<MessageId::read_eeprom_request>;

struct ReadFromEEPromResponse : BaseMessage<MessageId::read_eeprom_response> {
    uint8_t serial_number;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromEEPromResponse {
        uint8_t serial_number = 0;
        body = bit_utils::bytes_to_int(body, limit, serial_number);
        return ReadFromEEPromResponse{{}, serial_number};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(serial_number, body, limit);
        return iter - body;
    }
};

}  // namespace can_messages
