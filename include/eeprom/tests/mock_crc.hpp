#pragma once
#include <array>

#include "eeprom/firmware/crc16.h"

using namespace eeprom;

class MockCRC : public eeprom::CRC16Base {
  public:
    /**
     * Initialize crc module.
     */
    void crc16_init() {}

    /**
     * Compute the CRC
     * @param data Data
     * @param length Length of data
     * @return Computed CRC
     */
    uint16_t crc16_compute(const uint8_t* data, uint8_t length) {
        std::ignore = length;
        std::array<uint8_t, eeprom::types::book_data_length> data_byte{};
        std::memcpy(data_byte.data(), data, sizeof(*data));

        std::array<uint8_t, 2> crc_byte =
            calc_crc<eeprom::types::book_data_length>(data_byte);
        uint16_t crc;
        std::memcpy(&crc, crc_byte.data(), sizeof(crc));

        return crc;
    }

    /**
     * Continue accumulating CRC using provided data.
     * @param data Data
     * @param length Length of data
     * @return Accumulated CRC
     */
    uint16_t crc16_accumulate(const uint8_t* data, uint8_t length) {
        std::ignore = data;
        std::ignore = length;
        return 0;
    }

    /**
     * Reset the accumulated CRC value.
     */
    void crc16_reset_accumulator() {}

  private:
    // convert bitset to bytes
    template <uint16_t numbits>
    auto bitsettobytes(std::bitset<numbits> bits)
        -> std::array<uint8_t, numbits / 8> {
        std::array<uint8_t, numbits / 8> output{};

        for (int i = 0; i < numbits; i++) {
            uint8_t& cur = output[i / 8];

            if (bits.test(i)) {
                cur |= 1 << (i % 8);
            }
        }

        return output;
    }

    // convert bytes to bitset
    template <size_t numbytes>
    auto bytestobitset(std::array<uint8_t, numbytes> data)
        -> std::bitset<8 * numbytes> {
        std::bitset<numbytes * 8> bits;

        for (int i = 0; i < static_cast<int>(numbytes); ++i) {
            uint8_t cur = data[i];
            int offset = i * 8;

            for (int bit = 1; bit <= 8; ++bit) {
                auto mask = 1 << (8 - bit);
                bool is_set = (cur & mask) != 0;

                bits[offset + (8 - bit)] = is_set;
            }
        }

        return bits;
    }

    template <size_t num_bytes>
    auto calc_crc(std::array<uint8_t, num_bytes> data)
        -> std::array<uint8_t, 2> {
        // convert data array into a bitset, to make bit manipulation easier
        auto data_bitset = bytestobitset<num_bytes>(data);
        std::bitset<17> generator(0b10001000000100001);
        constexpr uint16_t generator_position = 16;

        // left shit data to accomadate crc
        std::bitset<(num_bytes * 8) + static_cast<size_t>(generator_position)>
            bit_data;
        for (size_t i = 0; i < data_bitset.size(); i++) {
            bit_data[i] = data_bitset[i];
        }
        bit_data <<= generator_position;
        uint16_t data_position = bit_data.size() - 1;

        while (data_position >= generator_position) {
            if (!bit_data.test(data_position)) {
                data_position--;
                continue;
            }

            uint16_t difference = data_position - generator_position;
            std::bitset<(num_bytes * 8) +
                        static_cast<size_t>(generator_position)>
                divisor(generator.to_ullong());
            divisor <<= difference;
            bit_data ^= divisor;
            // data_position--;
        }

        // extract remainder from bit_data
        std::bitset<16> crc;

        for (int i = 15; i >= 0; i--) {
            crc[i] = bit_data[i];
        }

        // convert crc bitset back into byte array
        std::array<uint8_t, 2> crc_byte = bitsettobytes<16>(crc);
        return crc_byte;
    }
};
