#pragma once

namespace eeprom {
namespace types {

// 0-255
using address = uint8_t;

// 0-8
using data_length = uint8_t;

constexpr data_length max_data_length = 8;

using EepromData = std::array<uint8_t, max_data_length>;

constexpr uint16_t DEVICE_ADDRESS = 0xA0;

}  // namespace types
}  // namespace eeprom
