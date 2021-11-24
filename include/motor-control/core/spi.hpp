#pragma once

#include <array>
#include <cstdint>
#include "common/core/bit_utils.hpp"

namespace spi {

/**
 * Abstract SPI driver base class.
 */
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class TMC2130Spi {
  public:
    static constexpr size_t BufferSize = 5;
    using BufferType = std::array<uint8_t, BufferSize>;

    enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

    virtual ~TMC2130Spi() = default;

    /**
     * Transmit and receive.
     *
     * @param transmit The transmit buffer.
     * @param receive The receive buffer.
     */
    virtual void transmit_receive(const TMC2130Spi::BufferType& transmit,
                                  TMC2130Spi::BufferType& receive) = 0;


    /**
     * Fill a buffer with a command.
     *
     * @param buffer The buffer
     * @param mode The mode (WRITE or READ)
     * @param address The register's address
     * @param data The 32-bit data wor
     */
    static void build_command(TMC2130Spi::BufferType& buffer, TMC2130Spi::Mode mode, uint8_t address, uint32_t data) {
        auto* iter = buffer.begin();
        // Address is ored with the mode.
        address |= static_cast<uint8_t>(mode);
        // Write address into buffer
        iter = bit_utils::int_to_bytes(address, iter, buffer.end());
        // Write data into the buffer
        iter = bit_utils::int_to_bytes(data, iter, buffer.end());
    }

};

}  // namespace spi
