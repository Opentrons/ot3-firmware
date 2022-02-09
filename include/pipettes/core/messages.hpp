#pragma once

#include <functional>

#include "common/core/i2c.hpp"

namespace pipette_messages {

using Callback = std::function<void(uint8_t *, uint16_t)>;

struct WriteToI2C {
    uint16_t address;
    uint8_t *buffer;
    uint16_t size;
};

struct ReadFromI2C {
    uint16_t address;
    uint8_t *buffer;
    uint16_t size;
    Callback client_callback;
};

}  // namespace pipette_messages