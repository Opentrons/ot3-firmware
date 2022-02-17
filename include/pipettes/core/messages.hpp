#pragma once

#include <functional>

#include "common/core/i2c.hpp"

namespace pipette_messages {

using MaxMessageBuffer = std::array<uint8_t, 5>;
using Callback = std::function<void(const MaxMessageBuffer&)>;

struct WriteToI2C {
    uint16_t address;
    MaxMessageBuffer buffer;
};

struct ReadFromI2C {
    uint16_t address;
    MaxMessageBuffer buffer;
    Callback client_callback;
};

struct PollReadFromI2C {
    uint16_t address;
    int polling;
    MaxMessageBuffer buffer;
    Callback client_callback;
};

}  // namespace pipette_messages