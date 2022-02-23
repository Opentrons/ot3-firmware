#pragma once

#include <functional>

#include "common/core/i2c.hpp"

namespace pipette_messages {

using MaxMessageBuffer = std::array<uint8_t, 5>;
using SingleRegisterCallback = std::function<void(const MaxMessageBuffer&)>;
using MultiRegisterCallback = std::function<void(const MaxMessageBuffer&, const MaxMessageBuffer&, bool)>;

struct WriteToI2C {
    uint16_t address;
    MaxMessageBuffer buffer;
};

struct ReadFromI2C {
    uint16_t address;
    MaxMessageBuffer buffer;
    SingleRegisterCallback client_callback;
};

struct SingleRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    MaxMessageBuffer buffer;
    SingleRegisterCallback client_callback;
    int delay_ms;
};

struct MultiRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    MaxMessageBuffer register_buffer_1;
    MaxMessageBuffer register_buffer_2;
    MultiRegisterCallback client_callback;
    int delay_ms;
};

}  // namespace pipette_messages