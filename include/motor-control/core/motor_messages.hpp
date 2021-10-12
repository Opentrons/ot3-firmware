#pragma once

#include <cstdint>

typedef int32_t sq0_31;  // 0: signed bit,  1-31: fractional bits
typedef uint64_t
    q31_31;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

namespace motor_messages {

// This is the message format currently used in CAN for a move message.
// We should probably put it somewhere else, but I am keeping it here
// for now.
struct CanMove {
    uint32_t target_position;  // in mm
};

struct Move {
    q31_31 target_position;  // in steps
    sq0_31 velocity;
    sq0_31 acceleration;
};
}  // namespace motor_messages
