#pragma once

#include <array>

#include "accessor.hpp"
#include "addresses.hpp"
#include "task.hpp"

namespace eeprom {
namespace revision {

using RevisionType = std::array<uint8_t, addresses::revision_length>;

/**
 * Class that reads and writes serial numbers.
 * @tparam EEPromTaskClient client of eeprom task
 */
template <task::TaskClient EEPromTaskClient>
class RevisionAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, RevisionType,
                                      addresses::revision_address_begin> {
  public:
    explicit RevisionAccessor(EEPromTaskClient& eeprom_client,
                              accessor::ReadListener<RevisionType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, RevisionType,
              addresses::revision_address_begin>::EEPromAccessor(eeprom_client,
                                                                 listener) {}
};

}  // namespace revision
}  // namespace eeprom
