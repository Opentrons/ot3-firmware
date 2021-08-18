#pragma once
#include <span>

#include "stm32g4xx_hal_conf.h"

namespace i2c {
class I2C {
  public:
    uint8_t DEVICE_ADDRESS = 0x1;
    static constexpr auto BUFFER_SIZE = 1;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    I2C();
    void transmit(uint8_t value);
    void receive(BufferType& receive);

  private:
    static constexpr auto TIMEOUT = 0xFFFF;
    I2C_HandleTypeDef handle;
};
}  // namespace i2c
