#include "common/firmware/i2c_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/i2c.h"
#pragma GCC diagnostic pop

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

I2C::I2C() : handle(MX_I2C_Init()) {}

void I2C::transmit(uint8_t value) {
    HAL_I2C_Master_Transmit(static_cast<I2C_HandleTypeDef*>(handle),
                            DEVICE_ADDRESS, &value, 1, TIMEOUT);
}

void I2C::receive(BufferType& receive) {
    HAL_I2C_Master_Receive(static_cast<I2C_HandleTypeDef*>(handle),
                           DEVICE_ADDRESS, receive.data(), receive.max_size(),
                           TIMEOUT);
}
