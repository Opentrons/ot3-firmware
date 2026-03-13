#pragma once
#include <cstdint>
#include <queue>

#include "accessor.hpp"
#include "task.hpp"

namespace eeprom {
namespace ot_library_accessor {

template <std::size_t SIZE>
using DataBufferType = std::array<uint8_t, SIZE>;
using DataTailType =
    std::array<uint8_t, eeprom::addresses::lookup_table_tail_length>;

/**OTLibraryAccessor**/
template <task::TaskClient EEpromTaskClient,
          eeprom::types::address ot_library_begin>
class OTLibraryAccessor
    : public eeprom::accessor::EEPromAccessor<EEpromTaskClient, data_begin> {
  public:
    template <std::size_t SIZE>
    explicit OTLibraryAccessor(EEpromTaskClient& eeprom_client,
                               accessor::ReadListener& readListener,
                               DataBufferType<SIZE>& buffer)

        : accessor::EEPromAccessor<EEpromTaskClient,
                                   addresses::data_address_begin>::
              EEPromAccessor(
                  eeprom_client, readListener,
                  accessor::AccessorBuffer(buffer.begin(), buffer.end())){};

    void write_data(uint16_t key, uint16_t len) { std::ignore = key, len; }

  private:
    auto calculate_crc(uint16_t data) -> uint16_t {
        // TODO: figure out how to make a CRC
        std::ignore = data;
    }
};
}  // namespace ot_library_accessor
}  // namespace eeprom
