#pragma once
#include <iterator>
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
template <task::TaskClient EEpromTaskClient, eeprom::types::address data_begin>
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

    enum class AccessOptions { Initalize, AddBook };

    void execute(AccessOptions method) { std::ignore = method; }

  private:
    std::queue<AccessOptions> execution_queue;
    types::address self_end = addresses::ot_library_end;
    types::address self_begin = addresses::ot_library_begin;
};
}  // namespace ot_library_accessor
}  // namespace eeprom
