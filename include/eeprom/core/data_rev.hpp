#pragma once

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"

namespace eeprom {
namespace data_revision {

using DataRevisionType = std::array<uint8_t, addresses::data_revision_length>;

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
