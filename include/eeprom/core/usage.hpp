#pragma once

#include <array>

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"

namespace eeprom {
namespace usage {

using UsageType = std::array<uint8_t, addresses::usage_length>;

struct increase_usage_cb_param {
    void* instance;
    UsageType time_to_add;
};

template <task::TaskClient EEPromTaskClient>
class UsageAccessor
    : public accessor::EEPromAccessor<
          EEPromTaskClient, UsageType, addresses::usage_address_begin,
          addresses::usage_address_end, addresses::usage_length> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, UsageType, addresses::usage_address_begin,
        addresses::usage_address_end, addresses::usage_length>::EEPromAccessor;

  public:
    explicit UsageAccessor(EEPromTaskClient& eeprom_client,
                           accessor::ReadListener<UsageType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, UsageType, addresses::usage_address_begin,
              addresses::usage_address_end,
              addresses::usage_length>::EEPromAccessor(eeprom_client,
                                                       listener) {}

    auto increase_usage(const UsageType& usage) -> void {
        types::data_length amount_to_read;
        auto read_addr = addresses::usage_address_begin;
        auto bytes_remain = addresses::usage_length;
        const auto cb_param = new increase_usage_cb_param{this, usage};
        amount_to_read = std::min(bytes_remain, types::max_data_length);
        this->eeprom_client.send_eeprom_queue(
            eeprom::message::ReadEepromMessage{.memory_address = read_addr,
                                               .length = amount_to_read,
                                               .callback = increase_callback,
                                               .callback_param = cb_param});
    }

  private:
    /**
     * Handle a completed read that was triggered by a increase_usage_call.
     * @param msg The message
     */
    void increase_callback(const eeprom::message::EepromMessage& msg,
                           const UsageType& time_to_add) {
        uint32_t time_to_add_int, current_time;
        auto new_usage = UsageType{};
        std::ignore = bit_utils::bytes_to_int(time_to_add, time_to_add_int);
        std::ignore = bit_utils::bytes_to_int(msg.data, current_time);
        current_time += time_to_add_int;
        std::ignore =
            bit_utils::int_to_bytes(current_time, new_usage.begin(),
                                    new_usage.begin() + sizeof(UsageType));
        this->write(new_usage);
    }

    /**
     * Handle a completed read.
     * @param msg The message
     * @param param This pointer.
     */
    static void increase_callback(const eeprom::message::EepromMessage& msg,
                                  void* param) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto* cb_param = reinterpret_cast<increase_usage_cb_param*>(param);
        auto* self = reinterpret_cast<UsageAccessor*>(cb_param->instance);
        self->increase_callback(msg, cb_param->time_to_add);
    }
};

}  // namespace usage
}  // namespace eeprom
