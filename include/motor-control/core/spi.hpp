#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace spi {

template <class Spi>
concept SpiProtocol = requires(Spi spi_comms,
                               const std::array<uint8_t, 5>& transmit,
                               std::array<uint8_t, 5>& receive) {
    {spi_comms.transmit_receive(transmit, receive)};
};

}  // namespace spi
