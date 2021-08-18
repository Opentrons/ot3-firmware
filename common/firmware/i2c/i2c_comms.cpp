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
 * BUFFER_SIZE -> 5
 * TIMEOUT -> 0xFFFF
 *
 * Public:
 * send_command takes a transmit buffer (with the command and data
 * already formatted), a uint8_t empty data which will be modified by
 * the information received from the stepper driver and status which will also
 * be modified by information received from the driver.
 */

/*
 * Private Functions
 */

/*
 * Public Functions
 */

I2C::I2C() : handle(MX_I2C1_Init()) {}

void I2C::transmit(uint8_t value) {
    HAL_I2C_Master_Transmit(&handle, DEVICE_ADDRESS, &value, 1, TIMEOUT);
}

void I2C::receive(BufferType& receive) {
    HAL_I2C_Master_Receive(&handle, DEVICE_ADDRESS, receive.data(),
                           receive.max_size(), TIMEOUT);
}