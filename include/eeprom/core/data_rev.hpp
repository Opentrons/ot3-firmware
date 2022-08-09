#pragma once

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"

namespace eeprom {
namespace data_revision {

using DataRevisionType = std::array<uint8_t, addresses::data_revision_length>;

/**
 * Class that reads and writes data revision values
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
class DataRevAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient,
                                      addresses::data_revision_address_begin> {
    using accessor::EEPromAccessor<
        EEPromTaskClient,
        addresses::data_revision_address_begin>::EEPromAccessor;

  public:
    explicit DataRevAccessor(EEPromTaskClient& eeprom_client,
                             accessor::ReadListener& listener,
                             DataRevisionType& buff)
        : accessor::EEPromAccessor<EEPromTaskClient,
                                   addresses::data_revision_address_begin>::
              EEPromAccessor(
                  eeprom_client, listener,
                  accessor::AccessorBuffer(buff.begin(), buff.end())) {}
};

}  // namespace data_revision
}  // namespace eeprom
