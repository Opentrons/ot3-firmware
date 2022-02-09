#pragma once

#include <functional>

#include "common/core/i2c.hpp"

namespace pipette_messages {

using MaxBufferSize = std::array<uint8_t, 5>;
using Callback = std::function<void(MaxBufferSize)>;

struct WriteToI2C {
    uint16_t address;
    MaxBufferSize buffer;
    uint16_t size;
};

struct ReadFromI2C {
    uint16_t address;
    MaxBufferSize buffer;
    uint16_t size;
    Callback client_callback;
};

}  // namespace pipette_messages