#pragma once

#include <cstdint>

typedef int32_t sq0_31;   // velocity value
typedef int64_t sq32_31;  // position tracker

namespace motor_messages {
struct Move {
    sq32_31 target_position;
    sq0_31 velocity;
    sq0_31 acceleration;
};
}  // namespace motor_messages
