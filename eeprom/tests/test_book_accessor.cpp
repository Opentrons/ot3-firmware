#include <cstdint>

#include "catch2/catch.hpp"
#include "eeprom/core/book_accessor.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"
#include "eeprom/tests/mock_eeprom_listener.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    uint16_t read_counter = 0;

    void send_eeprom_queue(const task::TaskMessage& m) {
        if (const auto* read_message =
                std::get_if<message::OTLibraryReadMessage>(&m)) {
            // structure return message
            message::OTLibraryBookMessage to_be_sent =
                eeprom::message::OTLibraryBookMessage{};
            to_be_sent.memory_address = read_message->memory_address;
            to_be_sent.length = read_message->length;
            to_be_sent.message_index = 0;

            auto data_to_be_sent =
                std::array<uint8_t,
                           static_cast<size_t>(types::DataSize::BOOK)>{};

            // TODO Make Data that will be sent back from "EEPROM"
            // generate arrays that aren't the valid one

            // first in page, counter value 2
            data_to_be_sent[2] = 0b00000010;  // counter
            data_to_be_sent[9] = 0b00000010;  // value

            // second page in book, counter value 3
            data_to_be_sent[types::page_length + 2] = 0b00000011;
            data_to_be_sent[types::page_length + types::book_header_length] =
                0b00000011;

            // third (current) page in book, counter value 4
            data_to_be_sent[(types::page_length * 2) + 2] = 0b00000100;
            data_to_be_sent[types::page_length * 2] = 0b10000100;  // CRC
            data_to_be_sent[(types::page_length * 2) + 1] = 0b01000000;
            data_to_be_sent[(types::page_length * 2) +
                            types::book_header_length] = 0b00000100;

            // second page in book, counter value 3
            data_to_be_sent[(types::page_length * 3) + 2] = 0b00000001;
            data_to_be_sent[(types::page_length * 3) +
                            types::book_header_length] = 0b00000001;

            to_be_sent.data = data_to_be_sent;

            const message::OTReadResponseCallback callback =
                read_message->callback;

            callback(to_be_sent, read_message->callback_param);
        } else if (const auto* read_message =
                       std::get_if<message::ReadEepromMessage>(&m)) {
            auto resp = message::EepromMessage{};
            resp.data.fill(0);
            read_message->callback(resp, read_message->callback_param);
        }
    }
};

SCENARIO("Book Accessor can read data from EEPROM") {
    auto mock_client = MockEEPromTaskClient{};
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<1>();
    auto tail_accessor =
        eeprom::dev_data::DevDataTailAccessor<MockEEPromTaskClient>{
            mock_client};
    auto test_book_accessor =
        book_accessor::BookAccessor<MockEEPromTaskClient, 1>{
            mock_client, mock_listener, buffer, tail_accessor};

    uint16_t key = 0;
    uint16_t len = 1;
    uint16_t offset = 0;
    uint32_t message_index = 0;

    test_book_accessor.get_data(key, len, offset, message_index);

    // check that the value read is correct
    REQUIRE(buffer[0] == 0b00000100);
}
