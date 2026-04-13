#pragma once
#include <bitset>
#include <cstddef>
#include <cstdint>

#include "accessor.hpp"
#include "addresses.hpp"
#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace ot_library_accessor {

template <size_t SIZE>
using DataBufferType = std::array<std::byte, SIZE>;
using DataTailType =
    std::array<uint8_t, eeprom::addresses::lookup_table_tail_length>;

/*
 * The PageType represents a page on which data will exist. It will be used to
 * organise data during reads and writes.
 * */
template <size_t SIZE>
struct PageType {
    std::array<std::byte, 2> crc;
    uint8_t length;
    DataBufferType<SIZE> data;

    struct checkValid {
        uint8_t errors;
        uint16_t counter;
        uint8_t retire;
    };
};

/* Addresses are taken as Bit Arrays, Make sure these arrays are BigEngdian*/
template <task::TaskClient EEpromTaskClient>
class BookAccessor
    : public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin> {
  public:
    template <size_t SIZE>
    explicit BookAccessor(EEpromTaskClient& eeprom_client,
                          accessor::ReadListener& read_listener,
                          DataBufferType<SIZE>& buffer)
        : accessor::EEPromAccessor<EEpromTaskClient,
                                   addresses::ot_library_begin>(
              eeprom_client, read_listener,
              accessor::AccessorBuffer(buffer.begin(), buffer.end())){};

    template <size_t SIZE>
    void create_new_data_part(uint8_t key, uint16_t len,
                              std::array<std::byte, SIZE> data) {
        std::ignore = key, len, data;
    }

    template <size_t SIZE>
    void write_data(uint8_t key, uint16_t len,
                    std::array<std::byte, SIZE> data) {
        std::ignore = key, len, data;
    }

    // WRITE_DATA CONVENIENCE METHODS

    void get_data(uint8_t key, uint16_t len, uint16_t offset,
                  uint32_t message_index) {
        // figure otut what the offset and message_index stuff are for
    }

    // GET_DATA CONVENIENCE METHODS
  private:
    // fields, decide what they are
    // Add a tail accessor?

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
            bit_data(data_bitset.to_ullong());
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

    template <size_t SIZE>
    void write_callback(uint8_t key, uint16_t len,
                        std::array<std::byte, SIZE> data) {
        std::ignore = data;
    }
};
}  // namespace ot_library_accessor
}  // namespace eeprom
