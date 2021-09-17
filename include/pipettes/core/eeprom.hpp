#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "common/core/bit_utils.hpp"

/*
 * A template and helper functions describing
 */

namespace eeprom {
template <class I2C>
concept EEPromPolicy =
    requires(I2C i2c_comms, const uint8_t transmit,
             std::array<uint8_t, I2C::BUFFER_SIZE>& receive) {
    /** A static integer member */
    {I2C::BUFFER_SIZE} -> std::convertible_to<int>;
    {i2c_comms.transmit(transmit)};
    {i2c_comms.receive(receive)};
};

template <eeprom::EEPromPolicy EEPromWriter>
void write(EEPromWriter& writer, const uint8_t serial_number) {
    writer.transmit(serial_number);
}

template <eeprom::EEPromPolicy EEPromWriter>
uint8_t read(EEPromWriter& writer) {
    using BufferType = std::array<uint8_t, EEPromWriter::BUFFER_SIZE>;
    BufferType rxBuffer{0};
    uint8_t data = 0x0;

    writer.receive(rxBuffer);

    bit_utils::bytes_to_int(rxBuffer, data);
    return data;
}
}  // namespace eeprom
