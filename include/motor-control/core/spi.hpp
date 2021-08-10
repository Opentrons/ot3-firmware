#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace spi {

constexpr const size_t BufferSize = 5;

template <class Spi>
concept TMC2130Spi = requires(Spi spi_comms,
                              const std::array<uint8_t, BufferSize>& transmit,
                              std::array<uint8_t, BufferSize>& receive) {
    {spi_comms.transmit_receive(transmit, receive)};
};

}  // namespace spi
