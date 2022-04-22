#pragma once

namespace eeprom_types {

// 0-8192
using address = uint16_t;

// 0-32
using data_length = uint8_t;

constexpr data_length max_data_length = 32;

using EepromData = std::array<uint8_t, max_data_length>;

}  // namespace eeprom_types