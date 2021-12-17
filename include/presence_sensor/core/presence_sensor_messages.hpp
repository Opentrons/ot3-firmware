#pragma once

#include <cstdint>

#include "can/core/messages.hpp"

namespace presence_sensor_messages {

using um_per_tick = can_messages::um_per_tick;
using um_per_tick_sq = can_messages::um_per_tick_sq;



enum class AckMessageId : uint8_t { complete = 0x1, error = 0x04 };

struct Reading {
    uint8_t seq_id;
};

struct Ack_ {
   
    AckMessageId ack_id;
};

constexpr const int RADIX = 31;

}  // namespace presence_sensor_messages
