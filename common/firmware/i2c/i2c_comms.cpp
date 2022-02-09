#include "common/firmware/i2c_comms.hpp"

#include "common/core/i2c.hpp"

using namespace i2c;

/*
 * I2C wrapper class.
 *
 * Public:
 * master_transmit - send out a command to I2C
 * master_receive - receive data from the I2C line
 *
 *
 */
I2C::I2C(HAL_I2C_HANDLE handle) : handle{handle} {}

auto I2C::central_transmit(uint8_t* data, uint16_t size, uint16_t dev_address,
                           uint32_t timeout) -> bool {
    return hal_i2c_master_transmit(handle, dev_address, data, size, timeout);
}

auto I2C::central_receive(uint8_t* data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool {
    return hal_i2c_master_receive(handle, dev_address, data, size, timeout);
}