#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"


using namespace can_messages;

auto HeartbeatRequest::parse(const BodyType& body)
    -> HeartbeatRequest {
    return HeartbeatRequest{};
}

auto HeartbeatResponse::parse(const BodyType& body)
    -> HeartbeatResponse {
    return HeartbeatResponse{};
}


auto GetStatusResponse::parse(const BodyType& body) -> GetStatusResponse {
    uint8_t status = 0;
    uint32_t data = 0;

    auto iter = body.begin();

    iter = bit_utils::bytes_to_int(iter, status);
    bit_utils::bytes_to_int(iter, data);
    return GetStatusResponse{status, data};
}

auto SetSpeedRequest::parse(const BodyType& body) -> SetSpeedRequest {
    uint32_t speed = 0;

    auto iter = body.begin();
    bit_utils::bytes_to_int(iter, speed);

    return SetSpeedRequest{speed};
}


auto GetSpeedResponse::parse(const BodyType& body) -> GetSpeedResponse {
    uint32_t speed = 0;

    auto iter = body.begin();
    bit_utils::bytes_to_int(iter, speed);

    return GetSpeedResponse{speed};
}
