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
/*
** Command a count-limited number of I2C reads from an single register
** of a single address at a specific poll timing. The poll timing may
** end up slightly off because other polls may be in progress for this
** device.
**
** After each read is complete, it will be passed to handle_buffer().
** After all reads are complete, client_callback() will be called.
*/
struct SingleRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
};

/**
** Command a count-limited number of I2C reads from a pair of registers
** of a single address at a specific poll timing. The poll timing may
** end up slightly off because other polls may be in progress for this
** device.
**
** After each read is complete, it will be passed to handle_buffer().
** After all reads are complete, client_callback() will be called.
* */
struct MultiRegisterPollReadFromI2C {
    uint16_t address;
    int polling;
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer register_1_buffer;
    sensor_callbacks::MaxMessageBuffer register_2_buffer;
    sensor_callbacks::SendToCanFunctionTypeDef client_callback;
    sensor_callbacks::MultiBufferTypeDef handle_buffer;
};

/**
** Start, update, or stop continuous polling of a single register
** of an I2C address at a specific timing.
**
** If the poll_id has already been configured, receipt of this message
** will reconfigure that ongoing poll. If it has not, a new poll will
** be started.
**
** If the delay_ms is 0, the poll will not happen. Otherwise, it will
** reoccur at the commanded period.
**
** To stop an ongoing poll, send this message with the same poll_id and
** a delay_ms of 0.
**
** After each read completes, handle_read() will be called.
*/
struct ConfigureSingleRegisterContinuousPolling {
    uint32_t poll_id;
    uint16_t address;
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer buffer;
    sensor_callbacks::SingleBufferTypeDef handle_buffer;
};

/**
** Start, update, or stop continuous polling of multiple registers
** of an I2C address at a specific timing.
**
** If the poll_id has already been configured, receipt of this message
** will reconfigure that ongoing poll. If it has not, a new poll will
** be started.
**
** If the delay_ms is 0, the poll will not happen. Otherwise, it will
** reoccur at the commanded period.
**
** To stop an ongoing poll, send this message with the same poll_id and
** a delay_ms of 0.
**
** After each read completes, handle_read() will be called.
*/
struct ConfigureMultiRegisterContinuousPolling {
    uint32_t poll_id;
    uint16_t address;
    int delay_ms;
    sensor_callbacks::MaxMessageBuffer register_1_buffer;
    sensor_callbacks::MaxMessageBuffer register_2_buffer;
    sensor_callbacks::MultiBufferTypeDef handle_buffer;
};

};  // namespace pipette_messages
