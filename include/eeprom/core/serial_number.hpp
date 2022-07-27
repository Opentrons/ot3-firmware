#pragma once

#include "accessor.hpp"
#include "addresses.hpp"
#include "task.hpp"

namespace eeprom {
namespace serial_number {

using SerialNumberType = std::vector<uint8_t>;

/**
 * Class that reads and writes serial numbers.
 * @tparam EEPromTaskClient client of eeprom task
 */
template <task::TaskClient EEPromTaskClient>
class SerialNumberAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, SerialNumberType,
                                      addresses::serial_number_address_begin,
                                      addresses::serial_number_length> {
  public:
    explicit SerialNumberAccessor(
        EEPromTaskClient& eeprom_client,
        accessor::ReadListener<SerialNumberType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, SerialNumberType,
              addresses::serial_number_address_begin,
              addresses::serial_number_length>::EEPromAccessor(eeprom_client,
                                                               listener) {}
};

}  // namespace serial_number
}  // namespace eeprom
