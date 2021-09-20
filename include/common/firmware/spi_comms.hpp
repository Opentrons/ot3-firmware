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

  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
};
}  // namespace spi
