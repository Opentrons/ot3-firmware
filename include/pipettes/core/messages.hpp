#pragma once

#include "common/core/i2c.hpp"
#include "sensors/core/callback_types.hpp"

namespace pipette_messages {

struct WriteToI2C {
    uint16_t address;
    sensor_callbacks::MaxMessageBuffer buffer;
};

struct ReadFromI2C {
    uint16_t address;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SingleRegisterCallback* client_callback;
};

struct SingleRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SingleRegisterCallback* client_callback;
    int delay_ms;
};

struct MultiRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    sensor_callbacks::MaxMessageBuffer register_buffer_1;
    sensor_callbacks::MaxMessageBuffer register_buffer_2;
    sensor_callbacks::MultiRegisterCallback* client_callback;
    int delay_ms;
};

}  // namespace pipette_messages