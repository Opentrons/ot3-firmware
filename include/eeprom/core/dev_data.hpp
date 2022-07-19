#pragma once

#include <array>

#include "accessor.hpp"
#include "addresses.hpp"
#include "can/core/message_core.hpp"
#include "common/core/bit_utils.hpp"
#include "task.hpp"

namespace eeprom {
namespace dev_data {

// set the buffer to the max data length
using DataBufferType = std::array<uint8_t, can::message_core::MaxMessageSize>;
using DataTailType = std::array<uint8_t, addresses::lookup_table_tail_length>;

struct increase_data_cb_param {
    void* instance;
    DataTailType data_to_add;
};

enum TableAction { READ, WRITE, CREATE };

struct table_entry_action {
    uint16_t key;
    types::address addr;
    types::data_length len;
    TableAction action;
};

struct table_action_cb_param {
    table_entry_action action_cmd;
    void* self;
}

// helper class to handle reading writing the data_tail value
template <task::TaskClient EEPromTaskClient>
class DevDataTailAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, DataTailType,
                                      addresses::lookup_table_tail_begin> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, DataTailType,
        addresses::lookup_table_tail_begin>::EEPromAccessor;

  public:
    explicit DevDataAccessor(EEPromTaskClient& eeprom_client,
                             accessor::ReadListener<DataTailType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, DataTailType,
              addresses::lookup_table_tail_begin>::EEPromAccessor(eeprom_client,
                                                                  listener) {}

  public:
    auto increase_data_tail(const DataTailType& data_added) -> void {
        types::data_length amount_to_read;
        auto read_addr = addresses::lookup_table_tail_begin;
        auto bytes_remain = addresses::lookup_table_tail_length;
        const auto cb_param = new increase_data_cb_param{this, data_added};
        amount_to_read = std::min(bytes_remain, types::max_data_length);
        this->eeprom_client.send_eeprom_queue(
            eeprom::message::ReadEepromMessage{
                .memory_address = read_addr,
                .length = amount_to_read,
                .callback = increase_tail_callback,
                .callback_param = cb_param});
    }

    auto increase_data_tail(const types::data_length data_added) -> void {
        DataTailType typed_length;
        std::ignore = bit_utils::int_to_bytes(
            data_added, typed_length.begin(),
            typed_length.begin() + sizeof(DataTailType));
        increase_data_tail(typed_length);
    }

  private:
    /**
     * Handle a completed read that was triggered by a increase_usage_call.
     * @param msg The message
     */
    void increase_tail_callback(const eeprom::message::EepromMessage& msg,
                                const DataTailType& data_to_add) {
        uint16_t data_to_add_int, current_data_length;
        auto new_data_length = DataTailType{};
        std::ignore = bit_utils::bytes_to_int(data_to_add, data_to_add_int);
        std::ignore = bit_utils::bytes_to_int(msg.data, current_data_length);
        current_data_length += data_to_add_int;
        std::ignore = bit_utils::int_to_bytes(
            current_time, new_data_length.begin(),
            new_data_length.begin() + sizeof(DataTailType));
        this->write(new_data_length);
    }

    /**
     * Handle a completed read.
     * @param msg The message
     * @param param This pointer.
     */
    static void increase_tail_callback(
        const eeprom::message::EepromMessage& msg, void* param) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto* cb_param = reinterpret_cast<increase_usage_cb_param*>(param);
        auto* self = reinterpret_cast<DevDataAccessor*>(cb_param->instance);
        self->increase_callback(msg, cb_param->time_to_add);
    }
}

template <task::TaskClient EEPromTaskClient>
class DevDataAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, DataBufferType,
                                      addresses::data_address_begin>,
    : accessor::ReadListener<DataTailType> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, DataBufferType,
        addresses::data_address_begin>::EEPromAccessor;

  public:
    explicit DevDataAccessor(EEPromTaskClient& eeprom_client,
                             accessor::ReadListener<DataBufferType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, DataBufferType,
              addresses::data_address_begin>::EEPromAccessor(eeprom_client,
                                                             listener) {
        tail_accessor = new DevDataTailAccessor(eeprom_client, this);
        tail_accessor.start_read();
        eeprom_client.send_eeprom_queue(message::ConfigRequestMessage{
            .callback = config_req_callback, .callback_param = this});
    }

    void create_data_part(uint16_t key, uint16_t len) {
        if (tail_updated && config_updated) {
            //  if the key is zero we don't need to read the former address
            if (key == 0) {
                message::WriteEepromMessage write;
                write.memory_address = addresses::data_address_begin;
                write.length = 2 * config.addr_bytes;
                auto data_iter = write.data.begin();
                data_iter = bit_utils::int_to_bytes(
                    config.mem_size - action_cmd.len, data_iter,
                    data_iter + config.addr_bytes);
                data_iter = bit_utils::int_to_bytes(
                    len, data_iter, data_iter + config.addr_bytes);
                eeprom_client.send_eeprom_queue(write);
                tail_accessor.increase_data_tail(2 * config.addr_bytes);
                data_tail += 2 * config.addr_bytes;
            } else {
                auto action_cmd = table_entry_action {
                    .key = key, .addr = calculate_table_entry_start(key),
                    .length = len,
                    .data = DataBufferType{}.action = TableAction::CREATE
                }

                const auto cb_param =
                    table_action_cb_param{.action_cmd action_cmd, .self = this}
                    // call a read to the previous table entry so we know where
                    // to put the data
                    eeprom_client.send_eeprom_queue(message::ReadEepromMessage{
                        .addr = calculate_table_entry_start(key - 1),
                        .len = 2 * config.addr_bytes,
                        .callback = table_action_callback,
                        .param = &cb_param})
            }
        }  // todo else
    }

    void on_read(DataTailType& tail) {
        if (std::equal(tail, {0x00, 0x00})) {
            DataTailType init_tail;
            bit_utils::int_to_bytes(addresses::data_address_begin,
                                    init_tail.begin(),
                                    init_tail.begin() + sizeof(DataTailType));
            tail_accessor.write(init_tail);
        } else {
            std::ignore = bytes_to_int(
                tail.begin(), tail.begin() + sizeof(DataTailType), data_tail);
        }
        tail_updated = true;
    }

  private:
    DevDataTailAccessor tail_accessor;
    types::address data_tail;
    bool tail_updated{false};
    message::ConfigResponseMessage conf;
    bool config_updated{false};

    // callbacks
    void config_req_callback(message::ConfigResponseMessage& m) {
        conf = m;
        config_updated = true;
    }

    static void config_req_callback(message::ConfigResponseMessage& m,
                                    void* param) {
        auto* self =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<DevDataAccessor<EEPromTaskClient>*>(param);
        self->config_req_callback(m);
    }

    void table_action_callback(message::EepromMessage& m,
                               table_entry_action action_cmd) {
        switch (action_cmd.action) {
            // When we recive a message started from a create, the message will
            // contain the table entry for the previous key, we need this data
            // to assign the new key a storage location
            case TableAction::CREATE:
                types::address current_data_begin;
                auto data_iter = m.data.begin();
                data_iter = bit_utils::bytes_to_int(
                    data_iter, data_iter + config.addr_bytes,
                    current_data_begin);
                if (data_tail + action_cmd.len + (2 * config.addr_bytes) >
                    current_data_begin) {
                    LOG("Error attempted to iniztialze value too large for "
                        "memory");
                } else {
                    message::WriteEepromMessage write;
                    write.memory_address = data_tail;
                    write.length = 2 * config.addr_bytes;
                    data_iter = write.data.begin();
                    data_iter = bit_utils::int_to_bytes(
                        current_data_begin - action_cmd.len, data_iter,
                        data_iter + config.addr_bytes);
                    data_iter =
                        bit_utils::int_to_bytes(action_cmd.len, data_iter,
                                                data_iter + config.addr_bytes);
                    eeprom_client.send_eeprom_queue(write);
                    tail_accessor.increase_data_tail(2 * config.addr_bytes);
                    data_tail += 2 * config.addr_bytes;
                }
                break;
            case TableAction::READ:
                break;
            case TableAction::WRITE:
                LOG("ERROR, recieved a callback from a write command action "
                    "unknown");
                break;
        }
    }

    static void table_action_callback(message::EepromMessage& m, void* param) {
        auto* cb_param = reinterpret_cast<table_action_cb_param*>(param);
        auto* self = reinterpret_cast<DevDataAccessor*>(cb_param->self);
        self->table_action_callback(m, cb_param->action);
    }

    // helper function
    types::address calculate_table_entry_start(uint16_t key) {
        types::address addr;
        if (config_updated) {
            bit_utils::int_to_bytes(
                addresses::data_address_begin + (key * 2 * conf.addr_bytes),
                addr.begin(), addr.begin() + sizeof(types::address));
        }  // todo else
        return addr;
    }
};

}  // namespace dev_data
}  // namespace eeprom
