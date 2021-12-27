#include "common/firmware/i2c_comms.hpp"


using namespace i2c;


I2C::I2C(HAL_I2C_HANDLE handle): handle{handle} {}

bool I2C::master_transmit(uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout) {
    return hal_i2c_master_transmit(handle, DevAddress, data, size, timeout);
}

bool I2C::master_receive(uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout) {
    return hal_i2c_master_receive(handle,DevAddress, data, size, timeout);
}