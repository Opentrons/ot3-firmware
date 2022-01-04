#pragma once

#include <cstdint>

#include "common/core/i2c.hpp"

namespace eeprom {

class EEPromWriter {
  public:
    EEPromWriter(i2c::I2CDeviceBase& i2c_device) : i2c_device{i2c_device} {}

    void write(uint8_t serial_number) {
        i2c_device.master_transmit(DEVICE_ADDRESS, &serial_number, 1, TIMEOUT);
    }

    auto read() -> uint8_t {
        uint8_t serial_number = 0;
        i2c_device.master_receive(DEVICE_ADDRESS, &serial_number, 1, TIMEOUT);
        return serial_number;
    }

  private:
    static constexpr uint16_t DEVICE_ADDRESS = 0x1;
    static constexpr auto TIMEOUT = 0xFFFF;
    i2c::I2CDeviceBase& i2c_device;
};

}  // namespace eeprom
