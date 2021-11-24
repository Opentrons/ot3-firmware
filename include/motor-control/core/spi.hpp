#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace spi {

/**
 * Abstract SPI driver base class.
 */
class TMC2130Spi {
  public:
    static constexpr size_t BufferSize = 5;
    using BufferType = std::array<uint8_t, BufferSize>;

    /**
     * Transmit and receive.
     *
     * @param transmit The transmit buffer.
     * @param receive The receive buffer.
     */
    virtual void transmit_receive(const TMC2130Spi::BufferType& transmit,
                                  TMC2130Spi::BufferType& receive) = 0;

    virtual ~TMC2130Spi() {}
};

}  // namespace spi
