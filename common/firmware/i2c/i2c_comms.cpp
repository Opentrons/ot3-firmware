#include "common/firmware/i2c_comms.hpp"

#include "FreeRTOS.h"
#include "task.h"

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

auto i2c_comms::I2C::central_transmit(MaxMessageBuffer& buffer, uint16_t dev_address,
                           uint32_t timeout) -> bool {
    return hal_i2c_master_transmit(handle, dev_address, buffer.data(), buffer.size(), timeout);
}

auto i2c_comms::I2C::central_receive(MaxMessageBuffer& buffer, uint16_t dev_address,
                          uint32_t timeout) -> bool {
    return hal_i2c_master_receive(handle, dev_address, buffer.data(), buffer.size(), timeout);
}

auto i2c_comms::I2C::wait_during_poll(uint16_t delay) -> void { vTaskDelay(delay); }

auto i2c_comms::I2C::set_handle(HAL_I2C_HANDLE i2c_handle) -> void { handle = i2c_handle; }