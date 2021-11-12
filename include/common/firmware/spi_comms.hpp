#pragma once
#include <cstdint>
#include <span>

namespace spi {
struct SPI_interface{
  void* SPI_handle;
};
class Spi {
  public:
    static constexpr auto BUFFER_SIZE = 5;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    Spi();
    void transmit_receive(const BufferType& transmit, BufferType& receive);
  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
    struct SPI_interface SPI_intf;
};
}  // namespace spi
