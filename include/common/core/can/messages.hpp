#pragma once

#include <cstdint>
#include "arbitration_ids.hpp"

using namespace can_arbitration_ids;

namespace can_messages {

struct HeartbeatRequest {
    static const auto id = MessageId::heartbeat_request;

    static auto parse(const std::span<uint8_t> & body) {
        return HeartbeatRequest{};
    }
};


struct HeartbeatResponse {
    static const auto id = MessageId::heartbeat_response;

    static auto parse(const std::span<uint8_t> & body) {
        return HeartbeatResponse{};
    }
};


}
