#pragma once

#include "task.hpp"
#include <cstdint>
#include <array>
#include "types.hpp"

namespace eeprom {
namespace serial_number {

constexpr auto SERIAL_NUMBER_LENGTH = 12;

using SerialNumberType = std::array<uint8_t, SERIAL_NUMBER_LENGTH>;


template <task::TaskClient EEPromTaskClient>
class SerialNumberAccessor {
  public:
    explicit SerialNumberAccessor(EEPromTaskClient& eeprom_client) : eeprom_client{eeprom_client} {}

    /**
     * Begin a read of the serial number.
     */
    auto start_read() -> void {
//        auto msg = eeprom::message::ReadEepromMessage{
//            .memory_address = can_msg.address,
//            .length = can_msg.data_length,
//            .callback = callback,
//            .callback_param = this};
    }

    /**
     * Write serial number to eeprom
     */
    auto write(const SerialNumberType& sn) -> void {
        auto first_write = types::EepromData{};
        auto second_write = types::EepromData{};

        std::copy_n(sn.cbegin(), first_write.size(), first_write.begin());
        std::copy_n(sn.cbegin() + first_write.size(), sn.size() - first_write.size(), second_write.begin());

        eeprom_client.send_eeprom_queue(
         eeprom::message::WriteEepromMessage{
            .memory_address = 0,
            .length = first_write.size(),
            .data = first_write});

        eeprom_client.send_eeprom_queue(
            eeprom::message::WriteEepromMessage{
                .memory_address = first_write.size(),
                .length = static_cast<types::data_length>(sn.size() - first_write.size()),
                .data = second_write});
    }

  private:
    EEPromTaskClient& eeprom_client;
    SerialNumberType serial_number{};
};

}
}