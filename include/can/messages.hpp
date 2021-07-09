#pragma once

#include <cstdint>
#include <span>

#include "ids.hpp"

using namespace can_ids;

namespace can_messages {

struct HeartbeatRequest {
    static const auto id = MessageId::heartbeat_request;

    static auto parse(const std::span<uint8_t>& body) -> HeartbeatRequest;
};

struct HeartbeatResponse {
    static const auto id = MessageId::heartbeat_response;

    static auto parse(const std::span<uint8_t>& body) -> HeartbeatResponse;
};

}  // namespace can_messages
