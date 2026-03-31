#pragma once
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "dev_data.hpp"
#include "messages.hpp"
#include "task.hpp"
#include "types.hpp"

namespace eeprom {
namespace ot_library_accessor {

template <size_t SIZE>
using DataBufferType = std::array<std::byte, SIZE>;
using DataTailType =
    std::array<uint8_t, eeprom::addresses::lookup_table_tail_length>;
using TableAction = dev_data::TableAction;
using table_entry_action = dev_data::table_entry_action;

/*
 * The PageType represents a page on which data will exist. It will be used to
 * organise data during reads and writes.
 * */
template <size_t SIZE>
struct PageType {
    std::array<std::byte, 2> crc;
    uint8_t length;
    DataBufferType<SIZE> data;
    uint16_t counter;

    struct checkValid {
        uint8_t errors;
        uint8_t retire;
    };
};

struct BookAccessorIntermediate {
  protected:
    DataBufferType<64> intermediate_buffer;
};

/* Addresses are taken as Bit Arrays, Make sure these arrays are BigEngdian*/
template <task::TaskClient EEpromTaskClient, size_t SIZE>
class BookAccessor
    : public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin>,
      eeprom::accessor::ReadListener,
      BookAccessorIntermediate {
  public:
    explicit BookAccessor(EEpromTaskClient& eeprom_client,
                          accessor::ReadListener& read_listener,
                          DataBufferType<SIZE>& buffer)
        : read_listener(read_listener),
          buffer(buffer),
          BookAccessorIntermediate(),
          accessor::EEPromAccessor<EEpromTaskClient,
                                   addresses::ot_library_begin>(
              eeprom_client, *this,
              accessor::AccessorBuffer(intermediate_buffer.begin(),
                                       intermediate_buffer.end())) {
        ;
        eeprom_client.send_eeprom_queue(
            message::ConfigRequestMessage{config_req_callback, this});
    }

    void create_new_data_part(uint8_t key, uint16_t len,
                              std::array<std::byte, SIZE> data) {
        std::ignore = key, len, data;
    }

    void write_data(uint8_t key, uint16_t len,
                    std::array<std::byte, SIZE> data) {
        std::ignore = key, len, data;
    }

    // write WRITE_DATA CONVENIENCE METHODS

    void get_data(uint16_t key, uint16_t len, uint16_t offset,
                  uint32_t message_index) {
        if (read_write_ready()) {
            auto table_location = calculate_table_entry_start(key);
            if (table_location > tail_accessor.get_data_tail()) {
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
                .message_index = message_index,
                .memory_address = table_location,
                .length = static_cast<types::data_length>(2 * conf.addr_bytes),
                .callback = table_action_callback,
                .callback_param = this});
        }
    }

    auto read_write_ready() -> bool {
        return table_ready() && tail_accessor.data_rev_complete();
    }

    auto table_ready() -> bool {
        return config_updated && tail_accessor.get_tail_updated();
    }

    void read_complete(uint32_t message_index) override {
        // 1. save what's in buffer to FullRead.reads
        std::copy_n(intermediate_buffer.begin(), intermediate_buffer.size(),
                    check_read.reads[check_read.book_index]);
        // increment check_read
        check_read.book_index += 1;
        // 2. check if reads length >= 4
        //
        if (check_read.book_index < 4) {
            // turn a book address into a regular address
            auto read_address =
                static_cast<types::address>(check_read.book_address) << 8;

            types::address page_relative = 0;

            // move the address to read different pages inside book
            switch (check_read.book_index) {
                case 1:
                    page_relative = 0b0000000001000000;
                case 2:
                    page_relative = 0b0000000010000000;
                case 3:
                    page_relative = 0b0000000001000000;
            }
            read_address |= page_relative;

            this->start_read_at_offset(read_address, read_address + 64,
                                       message_index);
        } else {
            read_final(message_index);
        }
    }

    // GET_DATA CONVENIENCE METHODS
  private:
    struct FullRead {
        types::book_address book{};
        uint8_t book_index = 0;
        std::array<std::array<std::byte, SIZE>, 4> reads;
    };

    // fields, decide what they are
    // Add a tail accessor?
    dev_data::DevDataTailAccessor<EEpromTaskClient>& tail_accessor;
    message::ConfigResponseMessage conf = message::ConfigResponseMessage{};
    bool config_updated{false};
    table_entry_action action_cmd_m = dev_data::table_entry_action{};
    ReadListener& read_listener;
    DataBufferType<SIZE> buffer;

    FullRead check_read;

    auto calc_crc(std::array<std::byte, SIZE> data)
        -> std::array<std::byte, 2> {
        // convert data array into a bitset, to make bit manipulation easier
        auto data_bitset = bytestobitset<SIZE>(data);
        std::bitset<17> generator(0b10001000000100001);
        const uint16_t generator_position = 16;

        // left shit data to accomadate crc
        std::bitset<data_bitset.size() + generator_position> bit_data(
            data_bitset << generator_position);
        uint16_t data_position = bit_data.size() - 1;

        while (data_position >= generator_position) {
            if (!bit_data.test(data_position)) {
                data_position--;
                continue;
            }

            uint16_t difference = data_position - generator_position;
            std::bitset<bit_data.size()> divisor(generator);
            divisor <<= difference;
            bit_data ^= divisor;
            // data_position--;
        }

        // extract remainder from bit_data
        std::bitset<16> crc;

        for (int i = 15; i >= 0; i--) {
            crc[i] = bit_data[i];
        }

        // convert crc bitset back into byte array
        std::array<std::byte, 2> crc_byte = bitsettobytes<16>(crc);
        return crc_byte;
    }

    // convert bytes to bitset
    template <size_t numbytes>
    auto bytestobitset(std::array<std::byte, numbytes> data)
        -> std::bitset<8 * numbytes> {
        std::bitset<numbytes * 8> bits;

        for (int i = 0; i < numbytes; ++i) {
            std::byte cur = data[i];
            int offset = ((numbytes - i) * 8) - 1;

            for (int bit = 1; bit <= 8; ++bit) {
                auto mask = static_cast<std::byte>(1 << (8 - bit));
                bool is_set = (cur & mask) != std::byte{0};

                bits[offset] = is_set;
                --offset;  // move to next bit in b
            }
        }

        return bits;
    }

    auto check_crc(std::array<std::byte, 64> bytes) -> bool {
        // Grab CRC from byte array
        std::array<std::byte, 2> given_CRC = std::span(bytes).first(2);

        // calculate the CRC from the given data
        std::array<std::byte, 56> given_data = std::span(bytes).last(56);
        std::array<std::byte, 2> calculated_crc = calc_crc(given_data);

        return (calculated_crc == given_CRC);
    }

    void read_final(uint16_t message_index) {
        // create variables representing read page addresses
        long read_00 = 0;
        long read_01 = 0;
        long read_11 = 0;
        long read_10 = 0;

        // convert counter from bytes to longs
        size_t counter_end = sizeof(std::byte) * 2;

        std::memcpy(&read_00, check_read.reads[0][2], sizeof(std::byte) * 2);
        std::memcpy(&read_01, check_read.reads[1][2], sizeof(std::byte) * 2);
        std::memcpy(&read_11, check_read.reads[2][2], sizeof(std::byte) * 2);
        std::memcpy(&read_10, check_read.reads[3][2], sizeof(std::byte) * 2);

        // find maximum value
        long max = std::max({read_00, read_01, read_11, read_10});

        if (action_cmd_m.action == TableAction::READ) {
            std::array<std::byte, 56> data_for_return{};
            auto returned_data = std::span(check_read.reads);
            bool crc_valid = true;

            if (max == read_00) {
                returned_data = std::span(check_read.reads)[0].last(56);
                crc_valid = check_crc(check_read.reads[0]);

            } else if (max == read_01) {
                returned_data = std::span(check_read.reads)[1].last(56);
                crc_valid = check_crc(check_read.reads[0]);

            } else if (max == read_11) {
                returned_data = std::span(check_read.reads)[2].last(56);
                crc_valid = check_crc(check_read.reads[0]);

            } else if (max == read_10) {
                returned_data = std::span(check_read.reads)[3].last(56);
                crc_valid = check_crc(check_read.reads[0]);
            }

            if (crc_valid) {
                std::copy_n(returned_data.begin(), sizeof(returned_data),
                            this->buffer);
            } else {
                // TODO change to read most recent
                std::string error = "CRC DIDN'T MATCH";

                std::copy_n(error.begin(), error.size(), this->buffer);
            }

            // TODO: use the result of check_crc as a "gateway" for now
            // tell object that called the read that the read is avaiable
            read_listener.read_complete(message_index);
        } else if (action_cmd_m.action == TableAction::WRITE) {
            // TODO: finish this when writing
            return;
        }
    }

    // convert bitset to bytes
    auto bitsettobytes(std::bitset<SIZE> bits)
        -> std::array<std::byte, SIZE / 8> {
        std::array<std::byte, SIZE / 8> output{};

        for (int i = SIZE - 1; i >= 0; i--) {
            std::byte& cur = output[output.size() - 1 - (i / 8)];

            if (bits.test(i)) {
                cur |= static_cast<std::byte>(1 << (i % 8));
            }
        }

        return output;
    }

    void write_callback(uint8_t key, uint16_t len,
                        std::array<std::byte, SIZE> data) {
        std::ignore = data;
    }

    // Methods from DevDataAccessor

    // callbacks
    void config_req_callback(const message::ConfigResponseMessage& m) {
        conf = m;
        config_updated = true;
        tail_accessor.set_config(conf);
        tail_accessor.start_read(0);
    }

    static void config_req_callback(const message::ConfigResponseMessage& m,
                                    void* param) {
        auto* self =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<dev_data::DevDataAccessor<EEpromTaskClient>*>(
                param);
        self->config_req_callback(m);
    }
    // Calculates data's location on the lookup table
    auto calculate_table_entry_start(uint16_t key) -> types::address {
        types::address addr = 0;
        if (config_updated) {
            addr = addresses::data_address_begin + (key * 2 * conf.addr_bytes);
        }
        return addr;
    }

    void table_action_callback(const message::EepromMessage& m) {
        const auto* data_iter = m.data.begin();
        types::address data_addr = 0;
        types::data_length data_len = 0;
        data_iter = bit_utils::bytes_to_int(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            data_iter, data_iter + conf.addr_bytes, data_addr);
        data_iter = bit_utils::bytes_to_int(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            data_iter, data_iter + conf.addr_bytes, data_len);
        if (conf.chip == hardware_iface::EEPromChipType::MICROCHIP_24AA02T) {
            data_addr = data_addr >> hardware_iface::ADDR_BITS_DIFFERENCE;
            data_len = data_len >> hardware_iface::ADDR_BITS_DIFFERENCE;
        }

        switch (action_cmd_m.action) {
            case TableAction::INITALIZE:
                [[fallthrough]];
            case TableAction::CREATE:
                [[fallthrough]];
            case TableAction::WRITE:
                [[fallthrough]];
            case TableAction::READ:
                data_addr += action_cmd_m.offset;
                // if the read action has length 0, read the whole value
                // else only read as much as requested
                if (action_cmd_m.len != 0) {
                    data_len = action_cmd_m.len;
                }
                this->start_read_at_offset(data_addr, data_addr + 64,
                                           m.message_index);
                break;
        }
    }
};

}  // namespace ot_library_accessor
}  // namespace eeprom
