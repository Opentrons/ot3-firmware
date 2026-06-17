#include "eeprom/firmware/crc16.h"

#include <bitset>
#include <cstdint>
#include <vector>

extern "C" {
// methods don't do anything
void crc16_reset_accumulator() {}

uint16_t crc16_accumulate(const uint8_t* data, uint8_t length) {
    return crc16_compute(data, length);
}

void crc16_init(void) {}

uint16_t crc16_compute(const uint8_t* data, uint8_t length) {
    // Guard against zero-length data (like empty_data initialization)
    // to prevent underflows or out-of-bound indexing in the bitset logic
    if (length == 0 || data == nullptr) {
        return 0;
    }

    // 1. Reconstruct a byte array from the pointer to process via bitsets
    std::vector<uint8_t> data_vec(data, data + length);

    // 2. Translate bytes into your bitset layout
    std::bitset<2048> data_bitset;
    for (size_t i = 0; i < length; ++i) {
        for (size_t bit = 0; bit < 8; ++bit) {
            data_bitset[(i * 8) + bit] = (data_vec[i] >> bit) & 1;
        }
    }

    std::bitset<17> generator(0b10001000000100001);
    constexpr uint16_t generator_position = 16;

    // Left shift data to accommodate CRC remainder
    std::bitset<2048 + generator_position> bit_data;
    for (size_t i = 0; i < length * 8; i++) {
        bit_data[i] = data_bitset[i];
    }
    bit_data <<= generator_position;
    uint16_t data_position = (length * 8) + generator_position - 1;

    // 3. Perform the bitset polynomial division modulo-2
    while (data_position >= generator_position) {
        if (!bit_data.test(data_position)) {
            data_position--;
            continue;
        }

        uint16_t difference = data_position - generator_position;
        std::bitset<2048 + generator_position> divisor(generator.to_ullong());
        divisor <<= difference;
        bit_data ^= divisor;
    }

    // 4. Extract the 16-bit CRC remainder
    std::bitset<16> crc;
    for (int i = 15; i >= 0; i--) {
        crc[i] = bit_data[i];
    }

    // 5. Convert back to a uint16_t for the HAL API return type
    return static_cast<uint16_t>(crc.to_ullong());
}
}
