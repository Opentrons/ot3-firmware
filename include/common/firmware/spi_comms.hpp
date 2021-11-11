#pragma once
#include <cstdint>
#include <span>

namespace spi {
class Spi {
  public:
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    Spi();
    void transmit_receive(const BufferType& transmit, BufferType& receive);
    void* get_SPI_config();
    void set_SPI_config();

  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
    void* SPI_config;
};
}  // namespace spi
