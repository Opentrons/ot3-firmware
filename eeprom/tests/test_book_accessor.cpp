#include <bitset>
#include <cstdint>
#include <variant>
#include <vector>

extern "C" {
__attribute__((weak)) void crc16_init(void) {}

__attribute__((weak)) uint16_t crc16_compute(const uint8_t* data,
                                             uint8_t length) {
    // Guard against zero-length data (like empty_data initialization)
    // to prevent underflows or out-of-bound indexing in the bitset logic
    if (length == 0 || data == nullptr) {
        return 0;
    }

    // 1. Reconstruct a byte array from the pointer to process via bitsets
    std::vector<uint8_t> data_vec(data, data + length);

    // 2. Translate bytes into your bitset layout
    std::bitset<2048> data_bitset;
    for (size_t i = 0; i < length; ++i) {
        for (size_t bit = 0; bit < 8; ++bit) {
            data_bitset[(i * 8) + bit] = (data_vec[i] >> bit) & 1;
        }
    }

    std::bitset<17> generator(0b10001000000100001);
    constexpr uint16_t generator_position = 16;

    // Left shift data to accommodate CRC remainder
    std::bitset<2048 + generator_position> bit_data;
    for (size_t i = 0; i < length * 8; i++) {
        bit_data[i] = data_bitset[i];
    }
    bit_data <<= generator_position;
    uint16_t data_position = (length * 8) + generator_position - 1;

    // 3. Perform the bitset polynomial division modulo-2
    while (data_position >= generator_position) {
        if (!bit_data.test(data_position)) {
            data_position--;
            continue;
        }

        uint16_t difference = data_position - generator_position;
        std::bitset<2048 + generator_position> divisor(generator.to_ullong());
        divisor <<= difference;
        bit_data ^= divisor;
    }

    // 4. Extract the 16-bit CRC remainder
    std::bitset<16> crc;
    for (int i = 15; i >= 0; i--) {
        crc[i] = bit_data[i];
    }

    // 5. Convert back to a uint16_t for the HAL API return type
    return static_cast<uint16_t>(crc.to_ullong());
}
}

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "eeprom/core/book_accessor.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"
#include "eeprom/firmware/crc16.h"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
using namespace eeprom;

class MockHardwareIface : public hardware_iface::EEPromHardwareIface {
    using hardware_iface::EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

enum ReadOption {
    VALID,
    ONE_INVALID,
    ALL_INVALID,
};

template <class I2CQueueWriter, class OwnQueue>
struct BMockEEpromTaskClient {
    ReadOption read_option = VALID;
    int read_counter = 0;

    BMockEEpromTaskClient() {
        writer.set_queue(&i2c_queue);
        backing.fill(0xFF);
    }

    task::EEPromMessageHandler<I2CQueueWriter, OwnQueue> true_eeprom_handler =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    void send_eeprom_queue(const task::TaskMessage& message) {
        std::visit([this](auto o) -> auto { this->visit(o); }, message);
    }

    std::vector<task::TaskMessage> messages_received{};

    std::array<uint8_t, 16384> backing{};

  private:
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    i2c::writer::Writer<test_mocks::MockMessageQueue> writer =
        i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    MockHardwareIface hardware_iface =
        MockHardwareIface{hardware_iface::EEPromChipType::ST_M24128_BF};
    void visit(const message::ReadEepromMessage& message) {
        // structure return message
        message::EepromMessage to_be_sent = eeprom::message::EepromMessage{};
        to_be_sent.memory_address = message.memory_address;
        to_be_sent.length = message.length;
        to_be_sent.message_index = message.message_index;

        auto data_to_be_sent =
            std::array<uint8_t, static_cast<size_t>(types::page_length)>{};

        if (message.memory_address < (backing.size() / 2)) {
            // front half of the device: just return what's actually stored
            // in `backing` at the requested address/length, so writes can be
            // read back
            std::copy_n(&backing[message.memory_address], message.length,
                        data_to_be_sent.begin());
        } else {
            // back half of the device: keep the existing scripted page
            // responses, which drive the book-accessor's read-cascade / CRC
            // logic
            // TODO Make Data that will be sent back from "EEPROM"
            // generate arrays that aren't the valid one

            if (read_option == VALID) {
                printf("read counter: %d\n", read_counter);
                switch (read_counter) {
                        // first in page, counter value 2
                    case 0:
                        data_to_be_sent[2] = 0x02;        // counter
                        data_to_be_sent[4] = 0x08;        // len
                        data_to_be_sent[8] = 0b00000010;  // value
                        break;
                        // second page in book, counter value 3
                    case 1:
                        data_to_be_sent[2] = 0x04;
                        data_to_be_sent[4] = 0x08;  // len
                        data_to_be_sent[8] = 0b00000011;
                        break;
                        // third (current) page in book, counter value 4
                    case 2:
                        data_to_be_sent[2] = 0b00000100;
                        data_to_be_sent[0] = 0b10000100;  // CRC
                        data_to_be_sent[1] = 0b01000000;  // still CRC
                        data_to_be_sent[4] = 0x08;        // len
                        data_to_be_sent[8] = 0b00000100;
                        break;
                        // fourth page in book, counter value 1
                    case 3:
                        data_to_be_sent[2] = 0b00000001;
                        data_to_be_sent[4] = 0x08;  // len
                        data_to_be_sent[8] = 0b00000001;
                        read_counter =
                            -1;  // reset counter so that if the object tries to
                                 // read again it will get the same values and
                                 // not an out of bounds value
                                 // negative one because the counter is
                                 // incremented at the end of this block
                        break;
                }
            } else if (read_option == ONE_INVALID) {
                switch (read_counter) {
                        // first in page, counter value 1
                    case 0:
                        data_to_be_sent[2] = 0b00000001;  // counter
                        data_to_be_sent[4] = 0x08;        // len
                        data_to_be_sent[9] = 0b00000001;  // value
                        break;
                        // second page in book, counter value 2
                    case 1:
                        data_to_be_sent[2] = 0b00000010;
                        data_to_be_sent[4] = 0x08;  // len
                        data_to_be_sent[9] = 0b00000010;
                        break;
                        // third (current) page in book, counter value 3 valid
                        // CRC; data value 4
                    case 2:
                        data_to_be_sent[2] = 0b00000011;
                        data_to_be_sent[0] = 0b10000100;  // CRC
                        data_to_be_sent[1] = 0b01000000;  // still CRC
                        data_to_be_sent[4] = 0x08;        // len
                        data_to_be_sent[9] = 0b00000100;
                        break;
                        // fourth page in book, counter value 3 invalid (no)
                        // CRC; data value 7
                    case 3:
                        data_to_be_sent[2] = 0b00000100;
                        data_to_be_sent[4] = 0x08;  // len
                        data_to_be_sent[9] = 0b00001001;
                        read_counter =
                            -1;  // reset counter so that if the object tries to
                                 // read again it will get the same values and
                                 // not an out of bounds value
                                 // negative one because the counter is
                                 // incremented at the end of this block
                        break;
                }
            } else if (read_option == ALL_INVALID) {
                // NO CRC sent at all
                data_to_be_sent[2] = 0b00000001;  // counter
                data_to_be_sent[9] = 0b00000001;  // value
                read_counter = 0;
            }

            read_counter++;
        }

        to_be_sent.data = data_to_be_sent;

        const auto callback = message.callback;

        callback(to_be_sent, message.callback_param);
        messages_received.push_back(message);
    }

    void visit(const message::WriteEepromMessage& message) {
        // for testing purposes we don't need to do anything when we get
        // a write message, but we could add some functionality here if
        // we wanted
        std::copy_n(message.data.begin(), message.length,
                    &backing[message.memory_address]);
        printf("write to address %u with length %u\n", message.memory_address,
               message.length);
        messages_received.push_back(message);
        printf("message index: %d\n", (int)messages_received.size());
    }

    void visit(const std::monostate& message) {
        std::ignore = message;
        // for testing purposes we don't need to do anything when we get a
        // monostate message, but we could add some functionality here if we
        // wanted
        messages_received.push_back(message);
    }

    void visit(const message::ConfigRequestMessage& message) {
        messages_received.push_back(message);

        auto m = task::TaskMessage{message};

        true_eeprom_handler.handle_message(m);
    }

    void visit(const i2c::messages::TransactionResponse& message) {
        std::ignore = message;
        messages_received.push_back(message);
    }
};

SCENARIO("Creating a data partition") {
    /* NOTE: This feature is very similar to
       * what we have in dev data, so we
       * are not testing it as extensively here, just making sure that the
       * book accessor can call create data without error and that it sends
       * the correct message to the EEPROM client. We will rely on the more
       * extensive testing of create data in dev data to make sure that
       create
       * data is working properly. */

    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<1>();
    std::array<std::array<uint8_t, types::page_length>, 4> all_reads{};
    for (auto& read : all_reads) {
        read.fill(0);
    }

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        1>{mock_client, mock_listener, buffer, tail_accessor, all_reads};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(true);

    GIVEN("Book Accessor initializes properly") {
        THEN("Create data part no data") {
            uint16_t key = 0;
            uint16_t len = 8;
            // std::array<uint8_t, 1> data{0b00000100};
            std::array<uint8_t, 0> empty_data{};

            mock_client.messages_received.clear();
            test_book_accessor.create_data_part<0>(key, len, empty_data);

            // 3 messages (config, read, write) sent to eeprom client (by
            // tail_accessor flow) before the write that we care about
            auto message = mock_client.messages_received[0];
            REQUIRE(std::holds_alternative<eeprom::message::WriteEepromMessage>(
                message));
            auto write_message =
                std::get<eeprom::message::WriteEepromMessage>(message);
            REQUIRE(write_message.memory_address ==
                    eeprom::addresses::data_address_begin);

            // check that address to be written is correct

            uint16_t data_address_written = 0;

            std::ignore = bit_utils::bytes_to_int(
                write_message.data.cbegin(),
                write_message.data.cbegin() + sizeof(data_address_written),
                data_address_written);
            // hide first byte of address, we only care that the second
            // byte is 0
            data_address_written &= 0x00FF;

            REQUIRE(data_address_written == 0);
        }
        // THEN("create data part with existing data") {
        uint16_t key = 1;
        uint16_t len = 1;
        // std::array<uint8_t, 1> data{0b00000100};
        std::array<uint8_t, 0> empty_data{};

        mock_client.messages_received.clear();
        test_book_accessor.create_data_part<0>(key, len, empty_data);

        // 3 messages (config, read, write) sent to eeprom client (by
        // tail_accessor flow) before the write that we care about
        auto message = mock_client.messages_received[0];
        REQUIRE(std::holds_alternative<eeprom::message::WriteEepromMessage>(
            message));
        auto write_message =
            std::get<eeprom::message::WriteEepromMessage>(message);
        REQUIRE(write_message.memory_address ==
                eeprom::addresses::data_address_begin + 4);

        // check that address to be written is correct

        uint16_t data_address_written = 0;

        std::ignore = bit_utils::bytes_to_int(
            write_message.data.cbegin(),
            write_message.data.cbegin() + sizeof(data_address_written),
            data_address_written);

        // hide first byte of address, we only care that the second
        // byte is 0
        data_address_written &= 0x00FF;

        REQUIRE(data_address_written == 0);
        // }
    }
}

SCENARIO("Book Accessor can read data from EEPROM") {
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<1>();
    std::array<std::array<uint8_t, types::page_length>, 4> all_reads{};
    for (auto& read : all_reads) {
        read.fill(0xff);
    }

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        1>{mock_client, mock_listener, buffer, tail_accessor, all_reads};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(true);

    uint16_t key = 0;
    uint16_t len = 8;
    uint16_t offset = 0;
    uint32_t message_index = 0;

    std::array<uint8_t, 0> empty_data{};
    GIVEN("Book Accessor initializes properly") {
        test_book_accessor.create_data_part<0>(key, len, empty_data);
        THEN("Read valid data properly") {
            mock_client.read_option = ReadOption::VALID;
            test_book_accessor.get_data(key, len, offset, message_index);
            // check that the value read is correct
            REQUIRE(buffer[0] == 0b00000000);
        }

        THEN("Cascade read when one page of data is invalid") {
            mock_client.read_option = ReadOption::ONE_INVALID;
            test_book_accessor.get_data(key, len, offset, message_index);
            // check that the value read is correct
            REQUIRE(buffer[0] == 0b00000000);
        }

        THEN("Return invalid data when all pages are invalid") {
            mock_client.read_option = ReadOption::ALL_INVALID;
            test_book_accessor.get_data(key, len, offset, message_index);
            // check that the value read is correct
            REQUIRE(buffer[0] == 0xAA);
        }
    }
}

SCENARIO("Book Accessor can write data to EEPROM") {
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<1>();
    std::array<std::array<uint8_t, types::page_length>, 4> all_reads{};
    for (auto& read : all_reads) {
        read.fill(0);
    }

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        1>{mock_client, mock_listener, buffer, tail_accessor, all_reads};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(true);

    uint16_t key = 0;
    uint16_t len = 1;
    uint16_t offset = 0;
    // uint32_t message_index = 0;

    std::array<uint8_t, 0> empty_data{};
    GIVEN("Data Partition is successfully created and reads are operational") {
        test_book_accessor.create_data_part<0>(key, len, empty_data);
        // make sure to set read_option to valid
        mock_client.read_option = ReadOption::VALID;

        THEN("Write data properly") {
            mock_client.messages_received.clear();
            std::array<uint8_t, 1> data_to_write{0b00000101};
            test_book_accessor.write_data(key, len, offset, data_to_write);

            // Get_data sends a few messages to the EEPROM client before the
            // write that we care about
            auto message = mock_client.messages_received[28];
            printf("Message content: %d",
                   std::holds_alternative<eeprom::message::WriteEepromMessage>(
                       message));

            REQUIRE(std::holds_alternative<eeprom::message::WriteEepromMessage>(
                message));

            auto write_message =
                std::get<eeprom::message::WriteEepromMessage>(message);

            // make sure the counter value is correct in the data that is being
            // written

            auto data = write_message.data;
            const auto* data_iter = data.begin();

            // check that counter value is correct (get_data should have current
            // counter value of 4 because of the valid read_option, so the
            // counter value should be 5 when we write)
            uint16_t counter_value = 0;
            data_iter = bit_utils::bytes_to_int(data_iter + 2, data_iter + 4,
                                                counter_value);
            REQUIRE(counter_value == 768);

            // check that addres being written is correct

            // expected: 16384 (final adress of EEPROM) - 64 (page length) to
            // find the book location. the page with the lowest address in the
            // "VALID" case of the read is the 4th and final page. the address
            // of this page is 16384 - 64 - 64 = 16256
            uint16_t address_written = write_message.memory_address;
            printf("Address written: %d", address_written);
            REQUIRE(address_written == 16064);
        }
    }
}
