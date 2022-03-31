#pragma once

#include "common/core/i2c.hpp"
#include "sensors/core/callback_types.hpp"

namespace pipette_messages {

/*
** Command an immediate I2C write to an address with no response.
*/
struct WriteToI2C {
    uint16_t address;
    sensor_callbacks::MaxMessageBuffer buffer;
};

/*
** Command a single immediate I2C read from an address. handle_buffer
** will be called on the response, and after the response is gathered,
** client_callback will be called.
*/
struct ReadFromI2C {
    uint16_t address;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
};

/*
** Command an immediate I2C read-write transaction with an address.
** handle_buffer will be called on the response.
*/
struct TransactWithI2C {
    uint16_t address;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
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
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer register_buffer_1;
    sensor_callbacks::MaxMessageBuffer register_buffer_2;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::MultiBufferTypeDef handle_buffer;
};

}  // namespace pipette_messages