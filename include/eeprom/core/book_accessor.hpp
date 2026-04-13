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
namespace book_accessor {

template <size_t SIZE>
using DataBufferType = std::array<uint8_t, SIZE>;
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
    std::array<uint8_t, 2> crc;
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
    DataBufferType<static_cast<size_t>(types::page_length * 4)>
        intermediate_buffer;
};

/*Accessor for OT Library. Takes byte arrays as data. Ensure they are in Little
 * Endian (in accordance with STM32 Architecture)
 *
 * SIZE is the size of the buffer*/

template <task::TaskClient EEpromTaskClient, size_t SIZE>
class BookAccessor
    : BookAccessorIntermediate,
      public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin>,
      eeprom::accessor::ReadListener {
  public:
    explicit BookAccessor(
        EEpromTaskClient& eeprom_client, accessor::ReadListener& read_listener,
        DataBufferType<SIZE>& buffer,
        dev_data::DevDataTailAccessor<EEpromTaskClient>& tail_accessor)
        : accessor::EEPromAccessor<EEpromTaskClient,
                                   addresses::ot_library_begin>(
              eeprom_client, *this,
              accessor::AccessorBuffer(intermediate_buffer.begin(),
                                       intermediate_buffer.end())),
          tail_accessor(tail_accessor),
          read_listener(read_listener),
          buffer(buffer) {
        eeprom_client.send_eeprom_queue(
            message::ConfigRequestMessage{config_req_callback, this});
    }

    // void create_new_data_part(uint8_t key, uint16_t len,
    //                           std::array<uint8_t, SIZE> data) {
    //     std::ignore = key, len, data;
    // }
    //
    // void write_data(uint8_t key, uint16_t len,
    //                 std::array<uint8_t, SIZE> data) {
    //     std::ignore = key, len, data;
    // }
    //
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
        return true;
        // return table_ready() && tail_accessor.data_rev_complete();
    }

    auto table_ready() -> bool {
        return config_updated && tail_accessor.get_tail_updated();
    }

    void read_complete(uint32_t message_index) override {
        // split big read into 4 pages
        for (uint8_t i = 0; i < 4; i++) {
            check_read.book_index = i;
            // 1. save what's in buffer to FullRead.reads
            std::copy_n((intermediate_buffer.begin() +
                         (static_cast<ptrdiff_t>(types::page_length * i))),
                        types::page_length,
                        check_read.reads[check_read.book_index].begin());
        }
        read_final(message_index);
    }

    // GET_DATA CONVENIENCE METHODS
  private:
    struct FullRead {
        uint8_t book_index = 0;
        std::array<std::array<uint8_t, types::page_length>, 4> reads{};
    };

    // fields, decide what they are
    // Add a tail accessor?
    dev_data::DevDataTailAccessor<EEpromTaskClient>& tail_accessor;
    message::ConfigResponseMessage conf = message::ConfigResponseMessage{};
    bool config_updated{false};
    table_entry_action action_cmd_m = dev_data::table_entry_action{};
    ReadListener& read_listener;
    DataBufferType<SIZE>& buffer;

    FullRead check_read{};

    // convert bitset to bytes
    template <uint16_t numbits>
    auto bitsettobytes(std::bitset<numbits> bits)
        -> std::array<uint8_t, numbits / 8> {
        std::array<uint8_t, numbits / 8> output{};

        for (int i = 0; i < numbits; i++) {
            uint8_t& cur = output[i / 8];

            if (bits.test(i)) {
                cur |= 1 << (i % 8);
            }
        }

        return output;
    }

    // convert bytes to bitset
    template <size_t numbytes>
    auto bytestobitset(std::array<uint8_t, numbytes> data)
        -> std::bitset<8 * numbytes> {
        std::bitset<numbytes * 8> bits;

        for (int i = 0; i < static_cast<int>(numbytes); ++i) {
            uint8_t cur = data[i];
            int offset = i * 8;

            for (int bit = 1; bit <= 8; ++bit) {
                auto mask = 1 << (8 - bit);
                bool is_set = (cur & mask) != 0;

                bits[offset + (8 - bit)] = is_set;
            }
        }

        return bits;
    }

    template <size_t num_bytes>
    auto calc_crc(std::array<uint8_t, num_bytes> data)
        -> std::array<uint8_t, 2> {
        // convert data array into a bitset, to make bit manipulation easier
        auto data_bitset = bytestobitset<num_bytes>(data);
        std::bitset<17> generator(0b10001000000100001);
        constexpr uint16_t generator_position = 16;

        // left shit data to accomadate crc
        std::bitset<(num_bytes * 8) + static_cast<size_t>(generator_position)>
            bit_data;
        for (size_t i = 0; i < data_bitset.size(); i++) {
            bit_data[i] = data_bitset[i];
        }
        bit_data <<= generator_position;
        uint16_t data_position = bit_data.size() - 1;

        while (data_position >= generator_position) {
            if (!bit_data.test(data_position)) {
                data_position--;
                continue;
            }

            uint16_t difference = data_position - generator_position;
            std::bitset<(num_bytes * 8) +
                        static_cast<size_t>(generator_position)>
                divisor(generator.to_ullong());
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
        std::array<uint8_t, 2> crc_byte = bitsettobytes<16>(crc);
        return crc_byte;
    }

    auto check_crc(std::array<uint8_t, types::page_length> bytes) -> bool {
        // Grab CRC from byte array
        std::array<uint8_t, 2> given_CRC{};
        std::copy_n(bytes.begin(), 2, given_CRC.begin());

        // calculate the CRC from the given data
        // Note: only the used bytes will be used in CRC caluclations
        std::array<uint8_t, types::book_data_length> given_data{};
        std::copy_n(bytes.begin() + types::book_header_length + 1,
                    action_cmd_m.len, given_data.begin());

        std::array<uint8_t, 2> calculated_crc = calc_crc(given_data);

        return (calculated_crc == given_CRC);
    }

    void read_final(uint16_t message_index) {
        // create variables representing read page addresses
        // TODO: Change names to reflect the fact that we are not doing one
        // large read instead of 4 small ones
        uint16_t read_00 = 0;
        uint16_t read_01 = 0;
        uint16_t read_11 = 0;
        uint16_t read_10 = 0;
        // convert counter from bytes to longs

        std::memcpy(&read_00, &check_read.reads[0][2], sizeof(read_00));
        std::memcpy(&read_01, &check_read.reads[1][2], sizeof(read_01));
        std::memcpy(&read_11, &check_read.reads[2][2], sizeof(read_11));
        std::memcpy(&read_10, &check_read.reads[3][2], sizeof(read_10));

        // find maximum value
        // TODO implement counter wraparound
        std::array<uint16_t, 4> reads = {read_00, read_01, read_11, read_10};
        std::sort(reads.begin(), reads.end(), std::greater<uint16_t>());
        uint16_t most_recent_index = 0;
        uint16_t most_recent_valid = reads[most_recent_index];

        if (action_cmd_m.action == TableAction::READ) {
            // std::array<uint8_t, 56> data_for_return{};
            types::data_length returned_data_len = action_cmd_m.len;
            auto returned_data =
                std::span(check_read.reads[0])
                    .subspan(types::book_header_length + 1, returned_data_len);
            bool crc_valid = false;

            while (!crc_valid) {
                // This while loop will keep looping through pages read until it
                // finds one whose written CRC matches the one calcluated
                // breaks if it has tried more than 4 times (the number of pages
                // in a book)
                if (most_recent_index >= 4) {
                    std::array<uint8_t,
                               static_cast<size_t>(types::DataSize::BOOK)>
                        error{0};
                    // writes an error to the buffer
                    // TODO ? maybe come up with a way to recover the data when
                    // this happens?

                    std::copy_n(error.begin(), error.size(),
                                this->buffer.begin());

                    return;
                }

                most_recent_valid = reads[most_recent_index];

                if (most_recent_valid == read_00) {
                    returned_data = std::span(check_read.reads[0])
                                        .subspan(types::book_header_length + 1,
                                                 returned_data_len);
                    crc_valid = check_crc(check_read.reads[0]);

                } else if (most_recent_valid == read_01) {
                    returned_data = std::span(check_read.reads[1])
                                        .subspan(types::book_header_length + 1,
                                                 returned_data_len);
                    crc_valid = check_crc(check_read.reads[1]);

                } else if (most_recent_valid == read_11) {
                    returned_data = std::span(check_read.reads[2])
                                        .subspan(types::book_header_length + 1,
                                                 returned_data_len);
                    crc_valid = check_crc(check_read.reads[2]);

                } else if (most_recent_valid == read_10) {
                    returned_data = std::span(check_read.reads[3])
                                        .subspan(types::book_header_length + 1,
                                                 returned_data_len);
                    crc_valid = check_crc(check_read.reads[3]);
                }

                most_recent_index++;
            }

            std::copy_n(returned_data.begin(), returned_data.size(),
                        this->buffer.begin());

            // tell object that called the read that the read is avaiable
            read_listener.read_complete(message_index);

        } else if (action_cmd_m.action == TableAction::WRITE) {
            // TODO: finish this when writing
            return;
        }
    }

    // void write_callback(uint8_t key, uint16_t len,
    //                     std::array<uint8_t, SIZE> data) {
    //     std::ignore = data;
    // }
    //
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
            reinterpret_cast<
                book_accessor::BookAccessor<EEpromTaskClient, SIZE>*>(param);
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
                // read all 4 whole pages at the same time
                this->OT_start_read_at_offset(
                    data_addr, data_addr + (types::page_length * 4),
                    m.message_index);
                break;
        }
    }

    static auto table_action_callback(const message::EepromMessage& m,
                                      void* param) -> void {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto* self = reinterpret_cast<BookAccessor*>(param);
        self->table_action_callback(m);
    }
};

}  // namespace book_accessor
}  // namespace eeprom
