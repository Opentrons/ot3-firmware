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
using DataBufferType = std::vector<uint8_t>;
using DataTailType = std::vector<uint8_t>;

struct increase_data_cb_param {
    void* instance;
    DataTailType data_to_add;
};

enum TableAction { READ, WRITE, CREATE, INITALIZE };

struct table_entry_action {
    uint16_t key;
    types::address offset;
    types::data_length len;
    TableAction action;
};

// helper class to handle reading writing the data_tail value
template <task::TaskClient EEPromTaskClient>
class DevDataTailAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, DataTailType,
                                      addresses::lookup_table_tail_begin,
                                      addresses::lookup_table_tail_length> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, DataTailType, addresses::lookup_table_tail_begin,
        addresses::lookup_table_tail_length>::EEPromAccessor;

  public:
    explicit DevDataTailAccessor(EEPromTaskClient& eeprom_client,
                                 accessor::ReadListener<DataTailType>& listener)
        : accessor::EEPromAccessor<EEPromTaskClient, DataTailType,
                                   addresses::lookup_table_tail_begin,
                                   addresses::lookup_table_tail_length>::
              EEPromAccessor(eeprom_client, listener) {}

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
        DataTailType typed_length =
            DataTailType(addresses::lookup_table_tail_length);
        std::ignore = bit_utils::int_to_bytes(
            data_added, typed_length.begin(),
            typed_length.begin() + addresses::lookup_table_tail_length);
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
        auto new_data_length =
            DataTailType(addresses::lookup_table_tail_length);
        std::ignore = bit_utils::bytes_to_int(data_to_add, data_to_add_int);
        std::ignore = bit_utils::bytes_to_int(msg.data, current_data_length);
        current_data_length += data_to_add_int;
        std::ignore = bit_utils::int_to_bytes(
            current_data_length, new_data_length.begin(),
            new_data_length.begin() + addresses::lookup_table_tail_length);
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
        auto* cb_param = reinterpret_cast<increase_data_cb_param*>(param);
        auto* self = reinterpret_cast<DevDataTailAccessor*>(cb_param->instance);
        self->increase_tail_callback(msg, cb_param->data_to_add);
    }
};

template <task::TaskClient EEPromTaskClient>
class DevDataAccessor
    : public accessor::EEPromAccessor<EEPromTaskClient, DataBufferType,
                                      addresses::data_address_begin,
                                      can::message_core::MaxMessageSize>,
      accessor::ReadListener<DataTailType> {
    using accessor::EEPromAccessor<
        EEPromTaskClient, DataBufferType, addresses::data_address_begin,
        can::message_core::MaxMessageSize>::EEPromAccessor;

  public:
    explicit DevDataAccessor(EEPromTaskClient& eeprom_client,
                             accessor::ReadListener<DataBufferType>& listener)
        : accessor::EEPromAccessor<
              EEPromTaskClient, DataBufferType, addresses::data_address_begin,
              can::message_core::MaxMessageSize>::EEPromAccessor(eeprom_client,
                                                                 listener),
          tail_accessor{DevDataTailAccessor<EEPromTaskClient>(
              eeprom_client,
              *(dynamic_cast<accessor::ReadListener<DataTailType>*>(this)))} {
        tail_accessor.start_read();
        eeprom_client.send_eeprom_queue(
            message::ConfigRequestMessage{config_req_callback, this});
    }

    void write_data(uint16_t key, uint16_t len, uint16_t offset,
                    std::vector<uint8_t> data) {
        if (tail_updated && config_updated) {
            auto table_location = calculate_table_entry_start(key);
            if (table_location > data_tail) {
                LOG("Error, attemping to read uninitalized value");
                return;
            }
            if (len > data.size()) {
                LOG("ERROR, trying to write %d bytes from a %lu byte buffer",
                    len, data.size());
                len = data.size();
            }
            LOG("Writing %d bytes to data partition", len);
            // copy the data to our internal buffer
            prep_buffer(len);
            std::copy_n(data.begin(), len, this->type_data.begin());
            this->action_cmd_m =
                table_entry_action{.key = key,
                                   .offset = offset,
                                   .len = len,
                                   .action = TableAction::WRITE};
            // call a read to the table entry so we know where
            // to put the data
            this->eeprom_client.send_eeprom_queue(message::ReadEepromMessage{
                .memory_address = table_location,
                .length = static_cast<types::data_length>(2 * conf.addr_bytes),
                .callback = table_action_callback,
                .callback_param = this});
        }
    }
    void write_data(uint16_t key, uint16_t len, std::vector<uint8_t> data) {
        write_data(key, len, 0, data);
    }
    void write_data(uint16_t key, std::vector<uint8_t> data) {
        write_data(key, data.size(), 0, data);
    }

    void get_data(uint16_t key, uint16_t len, uint16_t offset) {
        if (tail_updated && config_updated) {
            auto table_location = calculate_table_entry_start(key);
            if (table_location > data_tail) {
                LOG("Error, attemping to read uninitalized value");
                return;
            }
            action_cmd_m = table_entry_action{.key = key,
                                              .offset = offset,
                                              .len = len,
                                              .action = TableAction::READ};

            // call a read to the table entry so we know where
            // to read the data
            this->eeprom_client.send_eeprom_queue(message::ReadEepromMessage{
                .memory_address = table_location,
                .length = static_cast<types::data_length>(2 * conf.addr_bytes),
                .callback = table_action_callback,
                .callback_param = this});
        }
    }
    void get_data(uint16_t key, uint16_t len) { get_data(key, len, 0); }
    void get_data(uint16_t key) { get_data(key, 0, 0); }

    void create_data_part(uint16_t key, uint16_t len,
                          std::vector<uint8_t> data) {
        if (tail_updated && config_updated) {
            //  if the key is zero we don't need to read the former address
            if (key == 0) {
                message::WriteEepromMessage write;
                write.memory_address = addresses::data_address_begin;
                write.length = 2 * conf.addr_bytes;
                types::address new_ptr =
                    conf.mem_size - len - addresses::data_address_begin;
                if (conf.chip ==
                    hardware_iface::EEPromChipType::MICROCHIP_24AA02T) {
                    new_ptr = new_ptr << 8;
                    len = len << 8;
                }
                auto data_iter = write.data.begin();
                data_iter = bit_utils::int_to_bytes(
                    new_ptr, data_iter, data_iter + conf.addr_bytes);
                data_iter = bit_utils::int_to_bytes(
                    len, data_iter, data_iter + conf.addr_bytes);
                this->eeprom_client.send_eeprom_queue(write);
                tail_accessor.increase_data_tail(2 * conf.addr_bytes);
                data_tail += 2 * conf.addr_bytes;
                if (data.size() > 0) {
                    if (data.size() > len) {
                        LOG("Warning, sent too much data to initalize, "
                            "truncating to %d",
                            len);
                    }
                    this->write_at_offset(data, new_ptr, len);
                }
            } else {
                action_cmd_m =
                    table_entry_action{.key = key,
                                       .offset = 0,
                                       .len = len,
                                       .action = TableAction::CREATE};
                if (data.size() > 0) {
                    if (data.size() > len) {
                        LOG("Warning, sent too much data to initalize, "
                            "truncating to %d",
                            len);
                    }
                    prep_buffer(len);
                    std::copy_n(data.begin(), len, this->type_data.begin());
                    action_cmd_m.action = TableAction::INITALIZE;
                }
                // call a read to the previous table entry so we know where
                // to put the data
                this->eeprom_client.send_eeprom_queue(
                    message::ReadEepromMessage{
                        .memory_address = calculate_table_entry_start(key - 1),
                        .length = static_cast<types::data_length>(
                            2 * conf.addr_bytes),
                        .callback = table_action_callback,
                        .callback_param = this});
            }
        } else {
            LOG("ERROR, attempting to create data part before driver "
                "initalized");
        }
    }

    void create_data_part(uint16_t key, uint16_t len) {
        create_data_part(key, len, std::vector<uint8_t>{});
    }

    void on_read(const DataTailType& tail) {
        if (std::equal(tail.begin(),
                       tail.begin() + addresses::lookup_table_tail_length,
                       DataTailType{0x00, 0x00}.begin())) {
            DataTailType init_tail =
                DataTailType(addresses::lookup_table_tail_length);
            std::ignore = bit_utils::int_to_bytes(
                addresses::data_address_begin, init_tail.begin(),
                init_tail.begin() + addresses::lookup_table_tail_length);
            tail_accessor.write(init_tail);
            data_tail = addresses::data_address_begin;
        } else {
            std::ignore = bit_utils::bytes_to_int(
                tail.begin(),
                tail.begin() + addresses::lookup_table_tail_length, data_tail);
        }
        tail_updated = true;
    }

  private:
    DevDataTailAccessor<EEPromTaskClient> tail_accessor;
    types::address data_tail = 0;
    bool tail_updated{false};
    message::ConfigResponseMessage conf = message::ConfigResponseMessage{};
    bool config_updated{false};
    table_entry_action action_cmd_m = table_entry_action{};

    // callbacks
    void config_req_callback(const message::ConfigResponseMessage& m) {
        conf = m;
        config_updated = true;
    }

    static void config_req_callback(const message::ConfigResponseMessage& m,
                                    void* param) {
        auto* self =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<DevDataAccessor<EEPromTaskClient>*>(param);
        self->config_req_callback(m);
    }

    // this method gets called when the dev_data accessor reads the lookup table
    void table_action_callback(const message::EepromMessage& m) {
        auto data_iter = m.data.begin();
        types::address data_addr;
        types::data_length data_len;
        data_iter = bit_utils::bytes_to_int(
            data_iter, data_iter + conf.addr_bytes, data_addr);
        data_iter = bit_utils::bytes_to_int(
            data_iter, data_iter + conf.addr_bytes, data_len);
        if (conf.chip == hardware_iface::EEPromChipType::MICROCHIP_24AA02T) {
            data_addr = data_addr >> 8;
            data_len = data_len >> 8;
        }
        bool do_initalize = false;
        switch (action_cmd_m.action) {
            // When we recive a message started from a create, the message will
            // contain the table entry for the previous key, we need this data
            // to assign the new key a storage location
            case TableAction::INITALIZE:
                do_initalize = true;
                // don't break this is just an extension of create
                [[fallthrough]];
            case TableAction::CREATE:
                if (data_tail + action_cmd_m.len + (2 * conf.addr_bytes) >
                    data_addr) {
                    LOG("Error attempted to iniztialze value too large for "
                        "memory");
                } else {
                    message::WriteEepromMessage write;
                    write.memory_address = data_tail;
                    write.length = 2 * conf.addr_bytes;
                    auto write_iter = write.data.begin();
                    write_iter = bit_utils::int_to_bytes(
                        (data_addr - action_cmd_m.len), write_iter,
                        (write_iter + conf.addr_bytes));
                    write_iter =
                        bit_utils::int_to_bytes(action_cmd_m.len, write_iter,
                                                write_iter + conf.addr_bytes);
                    this->eeprom_client.send_eeprom_queue(write);
                    tail_accessor.increase_data_tail(2 * conf.addr_bytes);
                    data_tail += 2 * conf.addr_bytes;

                    if (do_initalize) {
                        this->write_at_offset(this->type_data,
                                              data_addr - action_cmd_m.len,
                                              data_addr);
                    }
                }
                break;
            case TableAction::READ:
                data_addr += action_cmd_m.offset;
                // if the read action has length 0, read the whole value else
                // only read as much as requested
                if (action_cmd_m.len != 0) {
                    data_len = action_cmd_m.len;
                }
                this->start_read_at_offset(data_addr, data_addr + data_len);
                break;
            case TableAction::WRITE:
                data_addr += action_cmd_m.offset;
                this->write_at_offset(this->type_data, data_addr,
                                      data_addr + action_cmd_m.len);
                break;
        }
    }

    static void table_action_callback(const message::EepromMessage& m,
                                      void* param) {
        auto* self = reinterpret_cast<DevDataAccessor*>(param);
        self->table_action_callback(m);
    }

    // helper function
    types::address calculate_table_entry_start(uint16_t key) {
        types::address addr = 0;
        if (config_updated) {
            addr = addresses::data_address_begin + (key * 2 * conf.addr_bytes);
        }  // todo else
        return addr;
    }

    void prep_buffer(uint16_t length) {
        this->type_data.clear();
        this->type_data.resize(length, 0x00);
    }
};

}  // namespace dev_data
}  // namespace eeprom
