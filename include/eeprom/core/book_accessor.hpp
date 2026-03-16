#pragma once
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
    uint16_t crc;
    uint8_t length;
    DataBufferType<SIZE> data;

    struct checkValid {
        uint8_t errors;
        uint16_t counter;
        uint8_t retire;
    };
};

template <task::TaskClient EEpromTaskClient>
class BookAccessor
    : public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin> {
  public:
    template <size_t SIZE>
    explicit BookAccessor(EEpromTaskClient& eeprom_client,
                          accessor::ReadListener& read_listener,
                          DataBufferType<SIZE>& buffer)
        : accessor::EEPromAccessor<eeprom_client, addresses::ot_library_begin>(
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

    template <size_t SIZE>
    auto calc_crc(std::array<std::byte, SIZE> data)
        -> std::array<std::byte, 2> {
        std::ignore = data;
    }

    template <size_t SIZE>
    void write_callback(uint8_t key, uint16_t len,
                        std::array<std::byte, SIZE> data) {
        std::ignore = data;
    }
};
}  // namespace ot_library_accessor
}  // namespace eeprom
