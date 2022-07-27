#pragma once

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"

namespace eeprom {
namespace data_revision {

using DataRevisionType = std::vector<uint8_t>;

template <task::TaskClient EEPromTaskClient>
class DataRevAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, DataRevisionType,
                                      addresses::data_revision_address_begin,
                                      addresses::data_revision_length> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, DataRevisionType,
        addresses::data_revision_address_begin,
        addresses::data_revision_length>::EEPromAccessor;

  public:
    explicit DataRevAccessor(EEPromTaskClient& eeprom_client,
                             accessor::ReadListener<DataRevisionType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, DataRevisionType,
              addresses::data_revision_address_begin,
              addresses::data_revision_length>::EEPromAccessor(eeprom_client,
                                                               listener) {}
};

}  // namespace data_revision
}  // namespace eeprom
