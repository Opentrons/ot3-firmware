#pragma once

namespace eeprom {
namespace types {

// 0-255
using address = uint8_t;

// 0-32
using data_length = uint8_t;

constexpr data_length max_data_length = 8;

using EepromData = std::array<uint8_t, max_data_length>;

}  // namespace types
}  // namespace eeprom