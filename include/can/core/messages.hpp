#pragma once

#include <cstdint>
#include <span>

#include "ids.hpp"

using namespace can_ids;

namespace can_messages {


using BodyType = std::span<uint8_t >;

/**
 * These types model the messages being sent and received over the can bus.
 *
 * The objects must implement the Parsable concept to deserialize the payload.
 */

struct HeartbeatRequest {
    static const auto id = MessageId::heartbeat_request;

    static auto parse(const BodyType& body) -> HeartbeatRequest;
};

struct HeartbeatResponse {
    static const auto id = MessageId::heartbeat_response;

    static auto parse(const BodyType& body) -> HeartbeatResponse;
};


struct StopRequest{
    static const auto id = MessageId::stop_request;

    static auto parse(const BodyType& body) -> StopRequest {
        return StopRequest{};
    }
};

struct GetStatusRequest{
    static const auto id = MessageId::get_status_request;

    static auto parse(const BodyType& body) -> GetStatusRequest {
        return GetStatusRequest{};
    }
};

struct GetStatusResponse{
    static const auto id = MessageId::get_status_response;
    uint8_t status;
    uint32_t data;

    static auto parse(const BodyType& body) -> GetStatusResponse;
};

struct MoveRequest{
    static const auto id = MessageId::move_request;

    static auto parse(const BodyType& body) -> MoveRequest {
        return MoveRequest{};
    }
};

struct SetupRequest{
    static const auto id = MessageId::setup_request;

    static auto parse(const BodyType& body) -> SetupRequest {
        return SetupRequest{};
    }
};

struct SetSpeedRequest{
    static const auto id = MessageId::set_speed_request;
    uint32_t mm_sec;

    static auto parse(const BodyType& body) -> SetSpeedRequest;
};

struct GetSpeedRequest{
    static const auto id = MessageId::get_speed_request;
    static auto parse(const BodyType& body) -> GetSpeedRequest {
        return GetSpeedRequest{};
    }
};

struct GetSpeedResponse{
    static const auto id = MessageId::get_speed_response;
    uint32_t mm_sec;

    static auto parse(const BodyType& body) -> GetSpeedResponse;
};

}  // namespace can_messages
