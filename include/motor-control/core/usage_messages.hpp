#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/types.hpp"

namespace usage_messages {

struct IncreaseDistanceUsage {
    uint16_t key;
    uint32_t distance_traveled_um;
};

struct IncreaseForceTimeUsage {
    uint16_t key;
    uint16_t seconds;
};

struct GetUsageRequest {
    uint32_t message_index;
    motor_hardware::UsageEEpromConfig usage_conf;
};

}  // namespace usage_messages