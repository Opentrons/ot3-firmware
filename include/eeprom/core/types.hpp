#pragma once

namespace eeprom {
namespace types {

// 0-65535
using address = uint16_t;

// 0-8
using data_length = uint16_t;

constexpr data_length max_data_length = 8;

using EepromData = std::array<uint8_t, max_data_length>;

constexpr uint16_t DEVICE_ADDRESS = 0xA0;

}  // namespace types
}  // namespace eeprom
