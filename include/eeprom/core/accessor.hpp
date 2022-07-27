#pragma once

#include <array>
#include <vector>

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

template <task::TaskClient EEPromTaskClient, typename T,
          types::address data_begin, types::address default_size>
class EEPromAccessor {
  public:
    explicit EEPromAccessor(EEPromTaskClient& eeprom_client,
                            ReadListener<T>& listener)
        : eeprom_client{eeprom_client},
          read_listener{listener},
          begin{data_begin} {}
    EEPromAccessor(const EEPromAccessor&) = delete;
    EEPromAccessor(EEPromAccessor&&) = delete;
    auto operator=(EEPromAccessor&&) -> EEPromAccessor& = delete;
    auto operator=(const EEPromAccessor&) -> EEPromAccessor& = delete;
    virtual ~EEPromAccessor() = default;
    /**
     * Begin a read of the data
     */
    auto start_read() -> void { start_read_at_offset(0, default_size); }

    /**
     * Write data to eeprom
     */
    auto write(const T& type_value) -> void {
        write_at_offset(type_value, 0, default_size);
    }

  protected:
    T type_data = T(default_size);
    EEPromTaskClient& eeprom_client;
    ReadListener<T>& read_listener;
    types::address begin;

    /**
     * Write data to eeprom at a specified offset
     */
    auto write_at_offset(const std::vector<uint8_t>& data,
                         types::data_length offset,
                         types::data_length limit_offset) -> void {
        types::data_length amount_to_write = 0;
        auto write = types::EepromData{};
        auto type_iter = data.begin();
        types::address write_addr = begin + offset;
        // NOLINTNEXTLINE(modernize-use-nullptr)
        while (type_iter < data.cend() && write_addr < (limit_offset + begin)) {
            amount_to_write = std::min(
                static_cast<types::data_length>(data.end() - type_iter),
                types::max_data_length);

            std::copy_n(type_iter, amount_to_write, write.begin());
            eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
                .memory_address = write_addr,
                .length = amount_to_write,
                .data = write});

            write_addr += amount_to_write;
            type_iter += amount_to_write;
        }
    }

    /**
     * Begin a read of the data at a specified offset
     */
    auto start_read_at_offset(types::data_length offset,
                              types::data_length limit_offset) -> void {
        // reset bytes_recieved to 0 so the response handler knows how much data
        // to wait for
        bytes_recieved = 0;
        bytes_to_read = limit_offset - offset;
        if (bytes_to_read > default_size) {
            LOG("error, attempting to read %lu bytes and max read size is "
                "%lu\n",
                bytes_to_read, default_size);
        }

        types::data_length amount_to_read = 0;
        types::address read_addr = begin + offset;
        types::data_length bytes_remain = (limit_offset + begin) - read_addr;

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

  private:
    /**
     * Handle a completed read.
     * @param msg The message
     */
    void callback(const eeprom::message::EepromMessage& msg) {
        // TODO (ryan 07-18-22) handle errors in response
        auto buffer_ptr = (type_data.begin() + (msg.memory_address - begin));
        std::copy_n(msg.data.cbegin(), msg.length, buffer_ptr);
        bytes_recieved += msg.length;
        if (bytes_recieved == bytes_to_read) {
            read_listener.on_read(
                T(type_data.begin(), type_data.begin() + bytes_recieved));
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
            reinterpret_cast<
                EEPromAccessor<EEPromTaskClient, T, data_begin, default_size>*>(
                param);
        self->callback(msg);
    }
    size_t bytes_recieved = 0;
    size_t bytes_to_read = 0;
};

}  // namespace accessor
}  // namespace eeprom
