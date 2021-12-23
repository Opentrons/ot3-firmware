#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "common/core/bit_utils.hpp"

/*
 * A template and helper functions describing
 */

class EEPromWriter {
  public:
    static constexpr uint16_t  DEVICE_ADDRESS = 0x1;
    static constexpr auto BUFFER_SIZE = 1;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;

    EEPromWriter();

    void transmit(uint8_t value);
    void receive(BufferType& receive);

  private:
    static constexpr auto TIMEOUT = 0xFFFF;
    void* handle;
};

void write(EEPromWriter& writer, const uint8_t serial_number) {
    writer.transmit(serial_number);
}

auto read(EEPromWriter& writer) -> uint8_t {
    using BufferType = std::array<uint8_t, EEPromWriter::BUFFER_SIZE>;
    BufferType rxBuffer{0};
    uint8_t data = 0x0;

    writer.receive(rxBuffer);

    bit_utils::bytes_to_int(rxBuffer, data);
    return data;
}
}  // namespace eeprom
