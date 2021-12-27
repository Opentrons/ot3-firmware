#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "common/core/i2c.hpp"

namespace eeprom {

class EEPromWriter {
  public:
    static constexpr uint16_t DEVICE_ADDRESS = 0x1;
    static constexpr auto BUFFER_SIZE = 1;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;

    EEPromWriter(i2c::I2CDeviceBase& i2c_device);

    void transmit(uint8_t value);
    void receive(BufferType& receive);

  private:
    static constexpr auto TIMEOUT = 0xFFFF;
    i2c::I2CDeviceBase& i2c_device;
};

void write(EEPromWriter& writer, const uint8_t serial_number);

auto read(EEPromWriter& writer) -> uint8_t;

}  // namespace eeprom
