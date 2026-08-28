#pragma once
#include <sys/types.h>

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "accessor.hpp"
#include "addresses.hpp"
#include "common/core/bit_utils.hpp"
#include "dev_data.hpp"
#include "messages.hpp"
#include "task.hpp"
#include "types.hpp"

extern "C" {
#include "eeprom/firmware/crc16.h"
}

namespace eeprom {
namespace book_accessor {

template <size_t SIZE>
using DataBufferType = std::array<uint8_t, SIZE>;
using DataTailType =
    std::array<uint8_t, eeprom::addresses::lookup_table_tail_length>;
using TableAction = dev_data::TableAction;
using table_entry_action = dev_data::table_entry_action;

static constexpr types::address read0_offset = 0;
static constexpr types::address read1_offset = types::page_length;
static constexpr types::address read2_offset = types::page_length * 2;
static constexpr types::address read3_offset = types::page_length * 3;

struct BookAccessorIntermediate {
  protected:
    DataBufferType<static_cast<size_t>(types::page_length)>
        intermediate_buffer{};
};
/*Accessor for OT Library. Takes byte arrays as data. Ensure they are in
 * Little Endian (in accordance with STM32 Architecture)
 *
 * SIZE is the size of the buffer*/

template <task::TaskClient EEpromTaskClient, uint16_t BUFFER_SIZE>
class BookAccessor
    : BookAccessorIntermediate,
      public eeprom::accessor::EEPromAccessor<EEpromTaskClient,
                                              addresses::ot_library_begin>,
      eeprom::accessor::ReadListener {
  public:
    explicit BookAccessor(
        EEpromTaskClient& eeprom_client, accessor::ReadListener& read_listener,
        DataBufferType<BUFFER_SIZE>& buffer,
        dev_data::DevDataTailAccessor<EEpromTaskClient>& tail_accessor)
        : BookAccessorIntermediate(),
          accessor::EEPromAccessor<EEpromTaskClient,
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

    template <size_t NUM_BYTES>
    void create_data_part(uint16_t key, uint16_t len,
                          std::array<uint8_t, NUM_BYTES>& data, bool migrating,
                          uint8_t data_flags = 0) {
        action_cmd_m.action = TableAction::CREATE;

        if (migrating) {
            action_cmd_m.action = TableAction::MIGRATE;
        }
        // set the length immediately. This is important for CRC calculations,
        // as the length of the data is not necessarily the same as the size of
        // the array passed in
        action_cmd_m.len = len;
        write_buffer_internal.data_flags = data_flags;
        write_buffer_internal.counter = 1;
        write_buffer_internal.length = len;
        std::fill(std::begin(write_buffer_internal.data), std::end(write_buffer_internal.data),
          0x00);
        if (!data.empty()) {
            if (data.size() > types::page_data) {
                LOG("Warning, sent too much data to initalize, "
                    "truncating to %d",
                    types::page_data);
            }
            std::copy_n(data.begin(), len, write_buffer_internal.data);
        }
        write_buffer_internal.crc = calc_crc(write_buffer_internal.data);
        if (table_ready()) {
            //  if the key is zero we don't need to read the former address
            if (key == 0) {
                // double check if this is writing to the data_table
                message::WriteEepromMessage write;
                write.memory_address = addresses::ot_library_table;
                write.length = 2 * conf.addr_bytes;
                // data pointers are offsets from the start of the data
                // section of the eeprom, so we subtract ot_library_begin
                // here to store the right value

                // new in OT library, subtract from ot_library_end to cut
                // off stale addresses
                types::address new_ptr =
                    addresses::ot_library_end -
                    (types::page_length * types::pages_per_book) -
                    addresses::ot_library_begin;

                // drop second byte (first byte is pre-aligned to 4 pages);
                new_ptr &= 0xFF00;

                auto* data_iter = write.data.begin();
                data_iter = bit_utils::int_to_bytes(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    new_ptr, data_iter, data_iter + conf.addr_bytes);
                data_iter = bit_utils::int_to_bytes(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    len, data_iter, data_iter + conf.addr_bytes);
                this->eeprom_client.send_eeprom_queue(write);

                if (!migrating) {
                    tail_accessor.increase_data_tail(2 * conf.addr_bytes);
                }
                this->write_at_offset(write_buffer, new_ptr, new_ptr + types::page_length, 0);
            } else {
                action_cmd_m.offset = 0;
                action_cmd_m.len = len;
                if (!data.empty()) {
                    if (!migrating) {
                        action_cmd_m.action = TableAction::INITALIZE;
                        // call a read to the previous table entry so we know
                        // where to put the data
                        tail_accessor.start_update();
                    }
                }

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

    template <size_t NUM_BYTES>
    void create_data_part(uint16_t key, uint16_t len,
                          std::array<uint8_t, NUM_BYTES>& data) {
        create_data_part(key, len, data, false);
    }

    void create_data_part(uint16_t key, uint16_t len) {
        auto dummy = std::array<uint8_t, 0>{};
        create_data_part(key, len, dummy);
    }

    template <std::size_t NUM_BYTES>
    void write_data(uint16_t key, uint16_t len, uint16_t offset,
                    std::array<uint8_t, NUM_BYTES>& data) {
        if (read_write_ready()) {
            if (len > types::page_data) {
                LOG("ERROR, trying to write %d bytes from a %lu byte "
                    "buffer",
                    len, types::page_data);
                len = types::page_data;
            }

            this->action_cmd_m =
                table_entry_action{.key = key,
                                   .offset = offset,
                                   .len = len,
                                   .action = TableAction::READ_BEFORE_WRITE};

            LOG("Writing %d bytes to data partition", types::page_length);

            write_buffer_internal.crc = calc_crc(data.begin());
            write_buffer_internal.length = len;
            // 0 out and copy into the write buffer
            std::fill(std::begin(write_buffer_internal.data), std::end(write_buffer_internal.data), 0);
            std::copy_n(data.begin(), len, write_buffer_internal.data);

            if (key == cached_key) {
                read_final(0);
            } else {
                // otherwise, call get_data and launch read flow
                get_data(key, len, offset, 0);
            }
        }
    }

    template <std::size_t NUM_BYTES>
    void write_data(uint16_t key, uint16_t len,
                    std::array<uint8_t, NUM_BYTES>& data) {
        write_data(key, len, 0, data);
    }

    template <std::size_t NUM_BYTES>
    void write_data(uint16_t key, std::array<uint8_t, NUM_BYTES>& data) {
        write_data(key, data.size(), 0, data);
    }

    void get_data(uint16_t key, uint16_t len, uint16_t offset,
                  uint32_t message_index) {
        if (read_write_ready()) {
            // reset all_reads
            for (auto& read : all_reads) {
                read.crc = 0;
                read.counter = 0;
                read.length = 0;
                read.reserved = 0;
                std::fill(std::begin(read.data), std::end(read.data), 0x00);
            }

            auto table_location = calculate_table_entry_start(key);

            if (!(action_cmd_m.action == TableAction::READ_BEFORE_WRITE)) {
                action_cmd_m = table_entry_action{.key = key,
                                                  .offset = offset,
                                                  .len = len,
                                                  .action = TableAction::READ};
            }

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

    void get_data(uint16_t key, uint16_t len, uint32_t message_index) {
        get_data(key, len, 0, message_index);
    }

    void get_data(uint16_t key, uint32_t message_index) {
        get_data(key, 0, 0, message_index);
    }

    auto read_write_ready() -> bool {
        return table_ready() && tail_accessor.data_rev_complete();
    }

    auto table_ready() -> bool {
        return config_updated && tail_accessor.get_tail_updated();
        // return true;
    }

    void read_complete(uint32_t message_index) override {
        // receives read data 4 times, once for each page. After the 4th time,
        // the data from all 4 pages is processed together and the final
        // callback is called with the processed data. This is because the book
        // is split into 4 pages and we need to read all 4 pages to get the full
        // data

        // save what's in buffer to all_reads
        std::copy_n(intermediate_buffer.begin(), types::page_length,
                    page_data_begin(all_reads[read_count]));

        // increment read_count
        read_count++;

        if (read_count < 4) {
            // kick off another read of the next page
            this->start_read_at_offset(
                current_book_address + (types::page_length * read_count),
                current_book_address + (types::page_length * (read_count + 1)),
                message_index);

        } else {
            read_count = 0;
            read_final(message_index);
        }
    }

    void set_testing(bool testing) { is_testing = testing; }

  private:
    // fields, decide what they are
    // Add a tail accessor?
    dev_data::DevDataTailAccessor<EEpromTaskClient>& tail_accessor;
    message::ConfigResponseMessage conf = message::ConfigResponseMessage{};
    bool config_updated{false};
    bool is_testing{false};
    table_entry_action action_cmd_m = dev_data::table_entry_action{};
    ReadListener& read_listener;
    uint8_t read_count = 0;
    DataBufferType<BUFFER_SIZE>& buffer;
    std::array<types::PageData, 4> all_reads = {};
    types::address current_book_address = addresses::ot_library_begin;
    types::PageData write_buffer_internal{};
    accessor::AccessorBuffer write_buffer{
        page_data_begin(write_buffer_internal),
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        page_data_begin(write_buffer_internal) + types::page_length};

    // cache the most recent key read to bypass reads wherever necesasry
    int16_t cached_key = -1;

    template <size_t num_bytes>
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    auto calc_crc(uint8_t data[num_bytes]) -> uint16_t {
        crc16_init();
        // pass the length of the data from action_cmd_m, not the size of the
        // array, since the array may be larger than the actual data being
        // written
        uint16_t crc =
            crc16_compute(data, static_cast<uint8_t>(action_cmd_m.len));

        return crc;
    }

    auto calc_crc(uint8_t* data) -> uint16_t {
        crc16_init();
        // pass the length of the data from action_cmd_m, not the size of the
        // array, since the array may be larger than the actual data being
        // written
        uint16_t crc =
            crc16_compute(data, static_cast<uint8_t>(action_cmd_m.len));

        return crc;
    }

    auto check_crc(types::PageData& page_data) -> bool {
        // if we're testing, we want to bypass CRC calculations to avoid having
        // too worry about hardware acceleration on incompatible platforms
        if (is_testing) {
            return true;
        }
        return (calc_crc<types::page_data>(page_data.data) == page_data.crc);
    }

    void read_final(uint16_t message_index) {
        // create variables representing read page addresses
        uint16_t read0 =
            all_reads[0].counter != 0xFFFF ? all_reads[0].counter : 0;
        uint16_t read1 =
            all_reads[1].counter != 0xFFFF ? all_reads[1].counter : 0;
        uint16_t read2 =
            all_reads[2].counter != 0xFFFF ? all_reads[2].counter : 0;
        uint16_t read3 =
            all_reads[3].counter != 0xFFFF ? all_reads[3].counter : 0;

        // find maximum value
        std::array<uint16_t, 4> reads = {read0, read1, read2, read3};

        // sort reads from largest to smallest
        std::sort(reads.begin(), reads.end(), std::greater<>());

        // handle counter wraparound
        if (reads[0] >= 65000) {
            // keep track of previous value to compute difference
            uint16_t prev = reads[0];

            for (auto& read : reads) {
                // check if previous read is close enough to current read to
                // be a non-wraparound value
                if (prev - read <= 1) {
                    prev = read;
                }
                // if not, then we have found the most recent value
                else {
                    uint16_t index = &read - &reads[0];
                    // re-arrange reads so most recent value is first
                    std::rotate(reads.begin(), reads.begin() + index,
                                reads.end());
                    break;
                }
            }
        }
        if (action_cmd_m.action == TableAction::READ) {
            find_most_recent(message_index, reads, read0, read1, read2, read3);
        }

        else if (action_cmd_m.action == TableAction::READ_BEFORE_WRITE) {
            // all reads[0] always has the length and length doesn't change so
            // copy it from there.
            write_buffer_internal.length = all_reads[0].length;
            find_next_write(reads, read0, read1, read2, read3);
        }
    }

    void find_most_recent(uint16_t message_index,
                          std::array<uint16_t, 4>& reads, uint16_t read0,
                          uint16_t read1, uint16_t read2, uint16_t read3) {
        action_cmd_m.len = all_reads[0].length;
        // set most recent index and most recent valid again
        uint16_t most_recent_index = 0;
        size_t all_reads_index = 0;
        uint16_t most_recent_valid = reads.at(most_recent_index);

        bool crc_valid = false;

        while (!crc_valid) {
            // This while loop will keep looping through pages read
            // until it finds one whose written CRC matches the one
            // calcluated breaks if it has tried more than 4 times (the
            // number of pages in a book)
            if (most_recent_index >= types::pages_per_book) {
                this->buffer.fill(0xAA);
                // writes an error to the buffer
                // TODO: ? maybe come up with a way to recover the data
                // when this happens?

                // tell object that called that the read is available, even
                // though it's just an error message, to avoid leaving it
                // hanging indefinitely or passing the wrong data
                read_listener.read_complete(message_index);
                return;
            }

            most_recent_valid = reads.at(most_recent_index);

            if (most_recent_valid == read0) {
                crc_valid = check_crc(all_reads[0]);
                all_reads_index = 0;

            } else if (most_recent_valid == read1) {
                crc_valid = check_crc(all_reads[1]);
                all_reads_index = 1;

            } else if (most_recent_valid == read2) {
                crc_valid = check_crc(all_reads[2]);
                all_reads_index = 2;

            } else if (most_recent_valid == read3) {
                crc_valid = check_crc(all_reads[3]);
                all_reads_index = 3;
            }

            most_recent_index++;
        }

        std::copy_n(all_reads.at(all_reads_index).data, BUFFER_SIZE,
                    this->buffer.begin());
        // cache the key that was just read so that if we need to do a write
        // right after we can bypass the read and just write to the same
        // place
        cached_key = action_cmd_m.key;

        // tell object that called the read that the read is avaiable
        read_listener.read_complete(message_index);
    }

    void find_next_write(std::array<uint16_t, 4>& reads, uint16_t read0,
                         uint16_t read1, uint16_t read2, uint16_t read3) {
        // create a new eeprom message to send to table_action_callback
        message::EepromMessage write_msg{};

        // because of the wraparound counter logic, we can be assured
        // that the last page is the least recently written page, so we
        // can use that to determine where to write the new data
        uint16_t least_recent = reads.at(reads.size() - 1);

        types::address page_address = current_book_address;

        // NOTE: this logic will break once a location eventually wears
        // out. It does not prevent writes to that location.

        if (least_recent == read0) {
            page_address += read0_offset;
        } else if (least_recent == read1) {
            page_address += read1_offset;
        } else if (least_recent == read2) {
            page_address += read2_offset;
        } else if (least_recent == read3) {
            page_address += read3_offset;
        }

        // clear write_msg.data just in case
        std::fill(std::begin(write_msg.data), std::end(write_msg.data), 0x00);
        // storing this in data instead of memory address because table
        // action callback cheks data to determine write location
        uint8_t* write_iter = write_msg.data.begin();
        // copy page address into first 2 bytes of data
        write_iter = bit_utils::int_to_bytes(
            page_address, write_iter,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            write_iter + conf.addr_bytes);
        // copy new counter value into next 2 bytes of data
        uint16_t new_counter = reads[0] + 1;
        if (new_counter > 65000) {
            // reset counter to avoid overflow, this will cause some
            // confusion in determining the most recent page, but it is
            // necessary to avoid counter overflow
            new_counter = 0;
        }
        write_buffer_internal.counter = new_counter;
        write_msg.length = conf.addr_bytes;
        // just fill memory address with beginning of lookup table tail
        write_msg.memory_address = addresses::lookup_table_tail_begin;

        // set table action to write
        action_cmd_m.action = TableAction::WRITE;

        cached_key = -1;

        table_action_callback(write_msg);
    }

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
                book_accessor::BookAccessor<EEpromTaskClient, BUFFER_SIZE>*>(
                param);
        self->config_req_callback(m);
    }
    // Calculates data's location on the lookup table
    auto calculate_table_entry_start(uint16_t key) -> types::address {
        types::address addr = 0;
        if (config_updated) {
            addr = addresses::ot_library_table + (key * 2 * conf.addr_bytes);
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
        bool do_initalize = false;
        bool migrating = false;
        switch (action_cmd_m.action) {
            case TableAction::MIGRATE:
                migrating = true;
                [[fallthrough]];
            case TableAction::INITALIZE:
                do_initalize = true;
                // don't break this is just an extension of create
                [[fallthrough]];
            case TableAction::CREATE:
                if (tail_accessor.get_data_tail() + types::page_length +
                        (2 * conf.addr_bytes) >
                    data_addr) {
                    LOG("Error attempted to initialize value too large for "
                        "memory");
                } else {
                    // First write the new table entry
                    message::WriteEepromMessage write;

                    // if we're migrating we want to write the new table
                    // entry to the same place as the old one, if we're not
                    // we want to write it to the tail
                    write.memory_address =
                        m.memory_address + (2 * conf.addr_bytes);

                    write.length = 2 * conf.addr_bytes;
                    auto* write_iter = write.data.begin();
                    uint16_t new_addr = data_addr - (types::page_length * 3);
                    // subtract a page to account for the fact that the
                    // final page of the EEPROM is off-limits
                    new_addr -= types::page_length;
                    write_iter = bit_utils::int_to_bytes(
                        new_addr, write_iter,
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                        write_iter + conf.addr_bytes);
                    write_iter = bit_utils::int_to_bytes(
                        action_cmd_m.len, write_iter,
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                        write_iter + conf.addr_bytes);
                    this->eeprom_client.send_eeprom_queue(write);

                    // After writing the table entry use the tail accessor
                    // to update the tail
                    if (!migrating) {
                        tail_accessor.increase_data_tail(2 * conf.addr_bytes);
                    }

                    // If we passed data into the create write that data
                    // into the memory
                    if (do_initalize) {
                        // initialize the first page
                        this->write_at_offset(write_buffer, new_addr,
                                              new_addr + types::page_length,
                                              m.message_index);
                    }
                }
                break;
            case TableAction::WRITE:
                data_addr += action_cmd_m.offset;
                this->write_at_offset(write_buffer, data_addr,
                                      data_addr + types::page_length,
                                      m.message_index);
                break;
            case TableAction::READ_BEFORE_WRITE:
                [[fallthrough]];
            case TableAction::READ:
                data_addr += action_cmd_m.offset;
                current_book_address = data_addr;
                this->start_read_at_offset(
                    data_addr, data_addr + types::page_length, m.message_index);
                break;
        }
    }

    static auto page_data_begin(types::PageData& pd) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<uint8_t*>(&pd);
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
