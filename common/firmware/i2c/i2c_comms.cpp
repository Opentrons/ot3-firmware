#include "common/firmware/i2c_comms.hpp"

#include "common/firmware/i2c.h"

using namespace i2c;

/*
 * I2C class and namespace.
 * Private:
 * transmit takes a reference to the transmit and receive
 * buffers. It will then initiate a SPI transmit/receive call.
 *
 * receive takes
 *
 * Global Private:
 * BUFFER_SIZE -> 1 (uint8_t)
 * TIMEOUT -> 0xFFFF
 *
 */

/*
 * TODO(LC 08/19/2021): We should set this configuration whenever an I2C
 * instance is created. Because we are currently not creating one, this should
 * be moved in a followup PR.
 * */

I2CConfig conf{
    .instance = REGISTER1, .address = 0x0, .address_mode = ADDRESSMODE_7BIT};

I2C::I2C() : handle(MX_I2C_Init(&conf)) {}

void I2C::transmit(uint8_t value) {
    HAL_I2C_Master_Transmit(&handle, DEVICE_ADDRESS, &value, 1, TIMEOUT);
}

void I2C::receive(BufferType& receive) {
    HAL_I2C_Master_Receive(&handle, DEVICE_ADDRESS, receive.data(),
                           receive.max_size(), TIMEOUT);
}