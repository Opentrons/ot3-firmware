#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace serial_number {

constexpr auto SERIAL_NUMBER_LENGTH = 12;

using SerialNumberType = std::array<uint8_t, SERIAL_NUMBER_LENGTH>;

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
        // The serial number might be larger than the amount that can be read
        // one shot. Create the number of reads required to read the full serial
        // number.
        for (types::address i = 0; i < SERIAL_NUMBER_LENGTH;
             i += types::max_data_length) {
            auto amount_to_read =
                std::min(static_cast<types::address>(SERIAL_NUMBER_LENGTH - i),
                         types::max_data_length);
            eeprom_client.send_eeprom_queue(
                eeprom::message::ReadEepromMessage{.memory_address = i,
                                                   .length = amount_to_read,
                                                   .callback = callback,
                                                   .callback_param = this});
        }
    }

    /**
     * Write serial number to eeprom
     */
    auto write(const SerialNumberType& sn) -> void {
        // The serial number might be larger than the amount that can be written
        // one shot. Create the number of writes messages required to
        // write the full serial number.
        for (types::address i = 0; i < sn.size(); i += types::max_data_length) {
            auto amount_to_write =
                std::min(static_cast<types::address>(sn.size() - i),
                         types::max_data_length);
            auto write = types::EepromData{};
            std::copy_n(sn.cbegin() + i, amount_to_write, write.begin());

            eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
                .memory_address = i, .length = amount_to_write, .data = write});
        }
    }

  private:
    static void callback(const eeprom::message::EepromMessage&, void*) {}
    EEPromTaskClient& eeprom_client;
    SerialNumberType serial_number{};
    Listener& read_listener;
};

}  // namespace serial_number
}  // namespace eeprom