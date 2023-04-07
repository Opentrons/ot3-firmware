#pragma once

#include <cstdint>

namespace usage_messages {

struct IncreaseDistanceUsage {
    uint16_t key;
    uint32_t distance_traveled_um;
};

struct GetUsageRequest {
    uint32_t message_index;
    uint16_t distance_usage_key;
};

}  // namespace usage_messages