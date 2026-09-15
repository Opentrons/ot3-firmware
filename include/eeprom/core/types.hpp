#pragma once

#include <array>
#include <cstdint>
namespace eeprom {
namespace types {

// 0-65535
using address = uint16_t;

// 0-8
using data_length = uint16_t;

constexpr data_length max_data_length = 8;

// OT-LIBRARY

// in the OT-LIBRARY data_length can be up to 56
constexpr data_length page_data = 56;
constexpr data_length book_header_length = 8;

constexpr data_length page_length = 64;

using EepromData = std::array<uint8_t, page_length / 2>;

constexpr uint8_t pages_per_book = 4;

constexpr uint8_t READ_ONLY = 0x1;

struct __attribute__((packed)) PageData {
    uint16_t crc = 0;
    uint16_t counter = 0;
    uint16_t length = 0;
    uint8_t data_flags = 0;
    uint8_t reserved = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    uint8_t data[types::page_data] = {0};
};

static_assert(sizeof(PageData) == types::page_length);

}  // namespace types
}  // namespace eeprom
