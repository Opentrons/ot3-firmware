#include "can/core/messages.hpp"

#include "common/core/bit_utils.hpp"

using namespace can_messages;

auto GetStatusResponse::parse(const BodyType& body) -> GetStatusResponse {
    uint8_t status = 0;
    uint32_t data = 0;

    auto iter = body.begin();
    auto limit = body.end();

    iter = bit_utils::bytes_to_int(iter, limit, status);
    iter = bit_utils::bytes_to_int(iter, limit, data);

    return GetStatusResponse{status, data};
}

void GetStatusResponse::serialize(BodyType& body) const {
    auto iter = body.begin();
    auto limit = body.end();
    iter = bit_utils::int_to_bytes(status, iter, limit);
    iter = bit_utils::int_to_bytes(data, iter, limit);
}

auto SetSpeedRequest::parse(const BodyType& body) -> SetSpeedRequest {
    uint32_t mm_sec = 0;
    auto iter = body.begin();
    iter = bit_utils::bytes_to_int(iter, body.end(), mm_sec);
    return SetSpeedRequest{mm_sec};
}

void SetSpeedRequest::serialize(BodyType& body) const {
    auto iter = body.begin();
    iter = bit_utils::int_to_bytes(mm_sec, iter, body.end());
}

auto GetSpeedResponse::parse(const BodyType& body) -> GetSpeedResponse {
    uint32_t mm_sec = 0;
    auto iter = body.begin();
    iter = bit_utils::bytes_to_int(iter, body.end(), mm_sec);
    return GetSpeedResponse{mm_sec};
}

void GetSpeedResponse::serialize(BodyType& body) const {
    auto iter = body.begin();
    iter = bit_utils::int_to_bytes(mm_sec, iter, body.end());
}
