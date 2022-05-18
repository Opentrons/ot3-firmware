#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "addresses.hpp"
#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace serial_number {

using SerialNumberType = std::array<uint8_t, addresses::serial_number_length>;

/**
 * Concept describing an object that listens for read serial numbers.
 * @tparam Listener
 */
template <typename Listener>
concept ReadListener = requires(Listener listener, const SerialNumberType& sn) {
    {listener.on_read(sn)};
};

/**
 * Class that reads and writes serial numbers.
 * @tparam EEPromTaskClient client of eeprom task
 * @tparam Listener listener for read serial numbers.
 */
template <task::TaskClient EEPromTaskClient, ReadListener Listener>
class SerialNumberAccessor {
  public:
    explicit SerialNumberAccessor(EEPromTaskClient& eeprom_client,
                                  Listener& listener)
        : eeprom_client{eeprom_client}, read_listener{listener} {}

    /**
     * Begin a read of the serial number.
     */
    auto start_read() -> void {
        auto amount_to_read = std::min(
            static_cast<types::address>(addresses::serial_number_length),
            types::max_data_length);
        eeprom_client.send_eeprom_queue(eeprom::message::ReadEepromMessage{
            .memory_address = addresses::serial_number_address_begin,
            .length = amount_to_read,
            .callback = callback,
            .callback_param = this});
    }

    /**
     * Write serial number to eeprom
     */
    auto write(const SerialNumberType& sn) -> void {
        auto amount_to_write = std::min(static_cast<types::address>(sn.size()),
                                        types::max_data_length);
        auto write = types::EepromData{};
        std::copy_n(sn.cbegin(), amount_to_write, write.begin());

        eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
            .memory_address = addresses::serial_number_address_begin,
            .length = amount_to_write,
            .data = write});
    }

  private:
    /**
     * Handle a completed read.
     * @param msg The message
     */
    void callback(const eeprom::message::EepromMessage& msg) {
        SerialNumberType serial_number{};
        std::copy_n(msg.data.cbegin(),
                    std::min(msg.data.size(), serial_number.size()),
                    serial_number.begin());
        read_listener.on_read(serial_number);
    }

    /**
     * Handle a completed read.
     * @param msg The message
     * @param param This pointer.
     */
    static void callback(const eeprom::message::EepromMessage& msg,
                         void* param) {
        auto* self =
            reinterpret_cast<SerialNumberAccessor<EEPromTaskClient, Listener>*>(
                param);
        self->callback(msg);
    }

    EEPromTaskClient& eeprom_client;
    Listener& read_listener;
};

}  // namespace serial_number
}  // namespace eeprom