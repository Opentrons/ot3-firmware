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
    int32_t velocity;
    int32_t acceleration;
    uint32_t motion_group_id;
    uint32_t sequence;
};

struct Move {
    q31_31 target_position;  // in steps
    sq0_31 velocity;
    sq0_31 acceleration;
    //    uint8_t group_id;
    //    uint8_t seq_id;
    //    uint32_t duration;  // in ticks
};

enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Ack {
    uint8_t group_id;
    uint8_t seq_id;
    AckMessageId ack_id;
};

}  // namespace motor_messages
