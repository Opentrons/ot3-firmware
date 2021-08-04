#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace spi {

template <class Spi, int BufferSize>
concept TMC2130Spi = requires(Spi spi_comms,
                              const std::array<uint8_t, BufferSize>& transmit,
                              std::array<uint8_t, BufferSize>& receive) {
    {spi_comms.transmit_receive(transmit, receive)};
};

}  // namespace spi
