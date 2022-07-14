#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "addresses.hpp"
#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace accessor {

/**
 * Interface of an object that listens for reads
 */
template <typename T>
class ReadListener {
  public:
    ReadListener() = default;
    ReadListener(const ReadListener&) = default;
    auto operator=(const ReadListener&) -> ReadListener& = default;
    ReadListener(ReadListener&&) noexcept = default;
    auto operator=(ReadListener&&) noexcept -> ReadListener& = default;

    virtual ~ReadListener() = default;
    virtual void on_read(const T&) = 0;
};

template <task::TaskClient EEPromTaskClient, typename T, types::address data_begin, types::address data_end, types::data_length data_length>
class EEPromAccessor {
  public:
    explicit EEPromAccessor(EEPromTaskClient& eeprom_client,
                                  ReadListener<T>& listener)
        : eeprom_client{eeprom_client}, read_listener{listener}, begin{data_begin}, end{data_end}, length{data_length} {}

    /**
     * Begin a read of the data
     */
    auto start_read() -> void {
        // reset bytes_recieved to 0 so the response handler knows how much data
        // to wait for
        bytes_recieved = 0;
        // clear the read buffer
        type_data.fill(0x00);

        types::data_length amount_to_read;
        auto read_addr = begin;
        auto bytes_remain = length;

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
     * Write data to eeprom
     */
    auto write(const T& type_value) -> void {
        types::data_length amount_to_write;
        auto write = types::EepromData{};
        auto type_iter = type_value.begin();
        auto write_addr = begin;

        while (type_iter < type_data.cend() && write_addr < (begin + type_data.size())) {
            amount_to_write =
                std::min(static_cast<types::data_length>(type_value.end() - type_iter),
                         types::max_data_length);

            std::copy_n(type_iter, amount_to_write, write.begin());
            printf("amnt to write %u, type_iter %p, remain? %lu\n", amount_to_write, type_iter, (type_data.end() - type_iter));
            eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
                .memory_address = write_addr,
                .length = amount_to_write,
                .data = write});

            write_addr += amount_to_write;
            type_iter += amount_to_write;
        }
    }

  private:
    /**
     * Handle a completed read.
     * @param msg The message
     */
    void callback(const eeprom::message::EepromMessage& msg) {
        auto buffer_ptr =
            (type_data.begin() +
             (msg.memory_address - begin));
        std::copy_n(msg.data.cbegin(), msg.length, buffer_ptr);
        bytes_recieved += msg.length;
        if (bytes_recieved == length) {
            read_listener.on_read(type_data);
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
            reinterpret_cast<EEPromAccessor<EEPromTaskClient, T, data_begin, data_end, data_length>*>(param);
        self->callback(msg);
    }
    T type_data{};
    size_t bytes_recieved = 0;
    EEPromTaskClient& eeprom_client;
    ReadListener<T>& read_listener;
    types::address begin;
    types::address end;
    types::data_length length;
};


} // namespace eeprom
} // namespace accessor
