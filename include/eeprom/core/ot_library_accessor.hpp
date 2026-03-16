#pragma once
#include <cstdint>

#include "accessor.hpp"
#include "addresses.hpp"
#include "task.hpp"

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

/**OTLibraryAccessor**/
template <task::TaskClient EEpromTaskClient,
          eeprom::types::address ot_library_begin>
class OTLibraryAccessor
    : public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin> {
  public:
    template <std::size_t SIZE>
    explicit OTLibraryAccessor(EEpromTaskClient& eeprom_client,
                               accessor::ReadListener& read_listener,
                               DataBufferType<SIZE>& buffer)
        : accessor::EEPromAccessor<EEpromTaskClient,
                                   addresses::ot_library_begin>::
              EEPromAccessor(
                  eeprom_client, read_listener,
                  accessor::AccessorBuffer(buffer.begin(), buffer.end())){};

    template <size_t SIZE>
    void write_data(uint16_t key, uint16_t len,
                    std::array<std::byte, SIZE> data) {
        PageType<SIZE> page;

        // Organise data to be sent to book
        uint16_t crc = calculate_crc(data);
        page.crc = crc;
        page.length = sizeof(data);
        page.data = data;

        // TODO Create book accessor and send this to book accessor to perform
        // actual write
    };

  private:
    auto calculate_crc(uint16_t data) -> uint16_t {
        // TODO: figure out how to make a CRC
        std::ignore = data;
    }
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
};
}  // namespace ot_library_accessor
}  // namespace eeprom
