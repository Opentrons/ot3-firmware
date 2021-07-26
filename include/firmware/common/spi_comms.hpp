#pragma once
#include <span>

#include "stm32g4xx_hal_conf.h"

namespace spi {
class Spi {
  public:
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    Spi();
    void transmit_receive(const BufferType& transmit, BufferType& receive);

  private:
    static constexpr auto TIMEOUT = 0xFFFF;
    SPI_HandleTypeDef handle;
};
}  // namespace spi
