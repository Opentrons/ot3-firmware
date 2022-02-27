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
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
};

struct SingleRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
};

struct MultiRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    sensor_callbacks::MaxMessageBuffer register_buffer_1;
    sensor_callbacks::MaxMessageBuffer register_buffer_2;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::MultiBufferTypeDef handle_buffer;
    int delay_ms;
};

}  // namespace pipette_messages