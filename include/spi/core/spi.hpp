#pragma once

#include <cstdint>

#include "common/core/bit_utils.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace hardware {

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

/**
 * Abstract SPI driver base class.
 */
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class SpiDeviceBase {
  public:
    virtual ~SpiDeviceBase() = default;

    /**
     * Transmit and receive.
     *
     * @param transmit The transmit buffer.
     * @param receive The receive buffer.
     */
    virtual auto transmit_receive(const utils::MaxMessageBuffer& transmit,
                                  utils::MaxMessageBuffer& receive,
                                  utils::ChipSelectInterface cs_intf)
        -> bool = 0;

    /**
     * Fill a buffer with a command.
     *
     * @param buffer The buffer
     * @param mode The mode (WRITE or READ)
     * @param address The register's address
     * @param data The 32-bit data wor
     */
    static void build_command(utils::MaxMessageBuffer& buffer, Mode mode,
                              uint8_t address, uint32_t data) {
        auto* iter = buffer.begin();
        // Address is ored with the mode.
        address |= static_cast<uint8_t>(mode);
        // Write address into buffer
        iter = bit_utils::int_to_bytes(address, iter, buffer.end());
        // Write data into the buffer
        iter = bit_utils::int_to_bytes(data, iter, buffer.end());  // NOLINT
    }
};

}  // namespace hardware

}  // namespace spi
