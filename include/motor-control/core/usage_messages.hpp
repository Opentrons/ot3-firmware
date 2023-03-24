#pragma once

#include <cstdint>

namespace usage_messages {

struct IncreaseDistanceUsage{
    uint16_t key;
    int32_t distance_traveled_um;
};

} // namespace usage_messages