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

struct HeartbeatRequest {
    static const auto id = MessageId::heartbeat_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> HeartbeatRequest {
        return HeartbeatRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct HeartbeatResponse {
    static const auto id = MessageId::heartbeat_response;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> HeartbeatResponse {
        return HeartbeatResponse{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct StopRequest {
    static const auto id = MessageId::stop_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> StopRequest {
        return StopRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct GetStatusRequest {
    static const auto id = MessageId::get_status_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetStatusRequest {
        return GetStatusRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct GetStatusResponse {
    static const auto id = MessageId::get_status_response;
    uint8_t status;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetStatusResponse {
        uint8_t status = 0;
        uint32_t data = 0;

        body = bit_utils::bytes_to_int(body, limit, status);
        body = bit_utils::bytes_to_int(body, limit, data);

        return GetStatusResponse{status, data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(status, body, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }
};

struct MoveRequest {
    static const auto id = MessageId::move_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MoveRequest {
        return MoveRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct SetupRequest {
    static const auto id = MessageId::setup_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetupRequest {
        return SetupRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct SetSpeedRequest {
    static const auto id = MessageId::set_speed_request;
    uint32_t mm_sec;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetSpeedRequest {
        uint32_t mm_sec = 0;
        body = bit_utils::bytes_to_int(body, limit, mm_sec);
        return SetSpeedRequest{mm_sec};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(mm_sec, body, limit);
        return iter - body;
    }
};

struct GetSpeedRequest {
    static const auto id = MessageId::get_speed_request;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetSpeedRequest {
        return GetSpeedRequest{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        return 0;
    }
};

struct GetSpeedResponse {
    static const auto id = MessageId::get_speed_response;
    uint32_t mm_sec;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetSpeedResponse {
        uint32_t mm_sec = 0;
        body = bit_utils::bytes_to_int(body, limit, mm_sec);
        return GetSpeedResponse{mm_sec};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(mm_sec, body, limit);
        return iter - body;
    }
};

}  // namespace can_messages
