#pragma once

#include <cstdint>

namespace motor_messages {
struct Move {
    uint32_t steps;
};
}  // namespace motor_messages
