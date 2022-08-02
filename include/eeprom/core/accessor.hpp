#pragma once

#include <array>
#include <cassert>
#include <vector>

#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace accessor {

class AccessorBuffer {
  public:
    AccessorBuffer() = default;
    template <typename B_ITER, typename L_ITER>
    requires bit_utils::ByteIterator<B_ITER> &&
        std::sentinel_for<B_ITER, L_ITER>
    explicit AccessorBuffer(B_ITER begin, L_ITER limit)
        : buffer_start(begin), buffer_limit(limit) {}
    auto size() -> size_t { return buffer_limit - buffer_start; }
    auto operator[](size_t key) -> const uint8_t& {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(buffer_start + key);
    }
    [[nodiscard]] auto begin() const -> uint8_t* { return buffer_start; }
    [[nodiscard]] auto end() const -> uint8_t* { return buffer_limit; }

  private:
    uint8_t* buffer_start;
    uint8_t* buffer_limit;
};

/**
 * Interface of an object that listens for reads
 */
class ReadListener {
  public:
    ReadListener() = default;
    ReadListener(const ReadListener&) = default;
    auto operator=(const ReadListener&) -> ReadListener& = default;
    ReadListener(ReadListener&&) noexcept = default;
    auto operator=(ReadListener&&) noexcept -> ReadListener& = default;

    virtual ~ReadListener() = default;
    virtual void read_complete() = 0;
};

template <task::TaskClient EEPromTaskClient, types::address data_begin>
class EEPromAccessor {
  public:
    explicit EEPromAccessor(EEPromTaskClient& eeprom_client,
                            ReadListener& listener, AccessorBuffer buff)
        : type_data(buff),
          eeprom_client{eeprom_client},
          read_listener(listener),
          begin{data_begin} {}
    EEPromAccessor(const EEPromAccessor&) = delete;
    EEPromAccessor(EEPromAccessor&&) = delete;
    auto operator=(EEPromAccessor&&) -> EEPromAccessor& = delete;
    auto operator=(const EEPromAccessor&) -> EEPromAccessor& = delete;
    virtual ~EEPromAccessor() = default;
    /**
     * Begin a read of the data
     */
    auto start_read() -> void { start_read_at_offset(0, type_data.size()); }

    /**
     * Write data to eeprom
     */
    auto write(const AccessorBuffer& type_value) -> void {
        write_at_offset(type_value, 0, type_data.size());
    }
    /**
     * Overload to write to preserve functionality
     */
    template <std::size_t SIZE>
    auto write(std::array<uint8_t, SIZE>& type_value) -> void {
        write(AccessorBuffer(type_value.begin(), type_value.end()));
    }

  protected:
    AccessorBuffer type_data;
    EEPromTaskClient& eeprom_client;
    ReadListener& read_listener;
    types::address begin;

    /**
     * Write data to eeprom at a specified offset
     */
    auto write_at_offset(const AccessorBuffer& data, types::data_length offset,
                         types::data_length limit_offset) -> void {
        types::data_length amount_to_write = 0;
        types::data_length write_remain = limit_offset - offset;
        auto write = types::EepromData{};
        auto* type_iter = data.begin();
        types::address write_addr = begin + offset;
        types::address limit_addr = begin + limit_offset;
        // NOLINTNEXTLINE(modernize-use-nullptr)
        while (type_iter < data.end() && write_addr < limit_addr) {
            amount_to_write = std::min(write_remain, types::max_data_length);

            std::copy_n(type_iter, amount_to_write, write.begin());
            eeprom_client.send_eeprom_queue(eeprom::message::WriteEepromMessage{
                .memory_address = write_addr,
                .length = amount_to_write,
                .data = write});

            write_addr += amount_to_write;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            type_iter += amount_to_write;
            write_remain -= amount_to_write;
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
        if (bytes_to_read > type_data.size()) {
            LOG("error, attempting to read %lu bytes and max read size is "
                "%lu\n",
                bytes_to_read, type_data.size());
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
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* buffer_ptr = (type_data.begin() + (msg.memory_address - begin));
        std::copy_n(msg.data.cbegin(), msg.length, buffer_ptr);
        bytes_recieved += msg.length;
        if (bytes_recieved == bytes_to_read) {
            read_listener.read_complete();
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
            reinterpret_cast<EEPromAccessor<EEPromTaskClient, data_begin>*>(
                param);
        self->callback(msg);
    }
    size_t bytes_recieved = 0;
    size_t bytes_to_read = 0;
};

}  // namespace accessor
}  // namespace eeprom
