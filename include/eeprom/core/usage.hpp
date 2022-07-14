#pragma once

#include <array>

#include "addresses.hpp"
#include "task.hpp"
#include "accessor.hpp"

namespace eeprom {
namespace usage {
    
using UsageType = std::array<uint8_t, addresses::usage_length>;

template <task::TaskClient EEPromTaskClient>
class UsageAccessor : public accessor::EEPromAccessor<EEPromTaskClient, UsageType, addresses::usage_address_begin, addresses::usage_address_end, addresses::usage_length> {
    using accessor::EEPromAccessor<EEPromTaskClient, UsageType, addresses::usage_address_begin, addresses::usage_address_end, addresses::usage_length>::EEPromAccessor;
    public:
        explicit UsageAccessor(EEPromTaskClient& eeprom_client,
                                      accessor::ReadListener<UsageType>& listener) : accessor::EEPromAccessor<EEPromTaskClient, UsageType, addresses::usage_address_begin, addresses::usage_address_end, addresses::usage_length>::EEPromAccessor(eeprom_client, listener) {}
};

}  // namespace usage
}  // namespace eeprom
