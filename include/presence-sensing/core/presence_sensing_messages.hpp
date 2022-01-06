#pragma once

#include <cstdint>

#include "can/core/messages.hpp"

namespace presence_sensing_messages {

enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Ack {
    AckMessageId ack_id;
};

struct GetVoltage{
    uint32_t z_motor;
    uint32_t a_motor;
    uint32_t gripper;

};

}  // namespace presence_sensing_messages