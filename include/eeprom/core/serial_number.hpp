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
 * Interface of an object that listens for read serial numbers.
 */
class ReadListener {
  public:
    ReadListener() = default;
    ReadListener(const ReadListener&) = default;
    auto operator=(const ReadListener&) -> ReadListener& = default;
    ReadListener(ReadListener&&) noexcept = default;
    auto operator=(ReadListener&&) noexcept -> ReadListener& = default;

    virtual ~ReadListener() = default;
    virtual void on_read(const SerialNumberType&) = 0;
};

/**
 * Class that reads and writes serial numbers.
 * @tparam EEPromTaskClient client of eeprom task
 */
template <task::TaskClient EEPromTaskClient>
class SerialNumberAccessor {
  public:
    explicit SerialNumberAccessor(EEPromTaskClient& eeprom_client,
                                  ReadListener& listener)
        : eeprom_client{eeprom_client}, read_listener{listener} {}

    /**
     * Begin a read of the serial number.
     */
    auto start_read() -> void {
        // reset bytes_recieved to 0 so the response handler knows how much data
        // to wait for
        bytes_recieved = 0;
        // clear the read buffer
        serial_number.fill(0x00);

        types::data_length amount_to_read;
        auto read_addr = addresses::serial_number_address_begin;
        auto bytes_remain = addresses::serial_number_length;

        while (bytes_remain > 0) {
            amount_to_read = std::min(bytes_remain, types::max_data_length);
            eeprom_client.send_eeprom_queue(
                eeprom::message::ReadEepromMessage{.memory_address = read_addr,
                                                   .length = amount_to_read,
                                                   .callback = callback,
                                                   .callback_param = this});
            bytes_remain -= amount_to_read;
            read_addr += amount_to_read;
        }
    }

    /**
     * Write serial number to eeprom
     */
    auto write(const SerialNumberType& sn) -> void {
        types::data_length amount_to_write;
        auto write = types::EepromData{};
        auto sn_iter = sn.begin();
        auto write_addr = addresses::serial_number_address_begin;

        while (sn_iter < sn.cend() && write_addr < sn.size()) {
            amount_to_write =
                std::min(static_cast<types::data_length>(sn.end() - sn_iter),
                         types::max_data_length);

            std::copy_n(sn_iter, amount_to_write, write.begin());
            eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
                .memory_address = write_addr,
                .length = amount_to_write,
                .data = write});

            write_addr += amount_to_write;
            sn_iter += amount_to_write;
        }
    }

  private:
    /**
     * Handle a completed read.
     * @param msg The message
     */
    void callback(const eeprom::message::EepromMessage& msg) {
        // the following line will evaluate to serial_number.begin() +
        // msg.memory_address with the current eeprom organization since the
        // address starts at 0 but doing this incase we move it later
        auto buffer_ptr =
            (serial_number.begin() +
             (msg.memory_address - addresses::serial_number_address_begin));
        std::copy_n(msg.data.cbegin(), msg.length, buffer_ptr);
        bytes_recieved += msg.length;
        if (bytes_recieved == addresses::serial_number_length) {
            read_listener.on_read(serial_number);
        }
    }

    /**
     * Handle a completed read.
     * @param msg The message
     * @param param This pointer.
     */
    static void callback(const eeprom::message::EepromMessage& msg,
                         void* param) {
        auto* self =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<SerialNumberAccessor<EEPromTaskClient>*>(param);
        self->callback(msg);
    }
    SerialNumberType serial_number{};
    size_t bytes_recieved = 0;
    EEPromTaskClient& eeprom_client;
    ReadListener& read_listener;
};

}  // namespace serial_number
}  // namespace eeprom
