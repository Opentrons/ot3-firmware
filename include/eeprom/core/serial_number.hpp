#pragma once

#include "accessor.hpp"
#include "addresses.hpp"
#include "task.hpp"

namespace eeprom {
namespace serial_number {

using SerialNumberType = std::array<uint8_t, addresses::serial_number_length>;
using SerialDataCodeType = std::array<uint8_t, 12>;
/**
 * Class that reads and writes serial numbers.
 *
 * Implementation Note: When creating an instance of this class, take care
 * to make the instance not outlive the backing buffer as this will cause a
 * SYSSEGV
 * The best way to implement this is to create an instance of both the accessor
 * and the backing buffer as members of a class so that they share scope
 *
 * @tparam EEPromTaskClient client of eeprom task
 **/
template <task::TaskClient EEPromTaskClient>
class SerialNumberAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient,
                                      addresses::serial_number_address_begin> {
  public:
    explicit SerialNumberAccessor(EEPromTaskClient& eeprom_client,
                                  accessor::ReadListener& listener,
                                  SerialNumberType& buffer)
        : accessor::EEPromAccessor<EEPromTaskClient,
                                   addresses::serial_number_address_begin>::
              EEPromAccessor(
                  eeprom_client, listener,
                  accessor::AccessorBuffer(buffer.begin(), buffer.end())) {}
};

}  // namespace serial_number
}  // namespace eeprom
