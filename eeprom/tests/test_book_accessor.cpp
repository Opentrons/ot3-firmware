#include <tuple>

extern "C" {
void vTaskDelay(const int x) { std::ignore = x; }
void vTaskDelete(void* x) { std::ignore = x; }
}

#include "catch2/catch.hpp"
#include "eeprom/core/book_accessor.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    uint16_t read_counter = 0;

    void send_eeprom_message(const task::TaskMessage& m) {
        if (const auto* read_message =
                std::get_if<eeprom::message::ReadEepromMessage>(&m)) {
            // structure return message
            message::OTLibraryMessage to_be_sent =
                eeprom::message::OTLibraryMessage<types::DataSize::BOOK>{};
            to_be_sent.memory_address = read_message->memory_address;
            to_be_sent.length = read_message->mem_size;
            to_be_sent.message_index = 0;

            auto data_to_be_sent =
                std::array<std::bytes, types::DataSize::BOOK>{};

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

            // const message::ReadResponseCallback callback =
            //     read_message->callback<types::DataSize::BOOK>;

            // callback(to_be_sent, read_message->callback_param);
        }
    }
};
