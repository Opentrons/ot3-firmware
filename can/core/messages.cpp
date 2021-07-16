#include "can/core/messages.hpp"

#include <span>

using namespace can_messages;

auto HeartbeatRequest::parse(const std::span<uint8_t>& body)
    -> HeartbeatRequest {
    return HeartbeatRequest{};
}

auto HeartbeatResponse::parse(const std::span<uint8_t>& body)
    -> HeartbeatResponse {
    return HeartbeatResponse{};
}
