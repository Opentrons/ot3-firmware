#include "catch2/catch.hpp"
#include "eeprom/core/serial_number.hpp"
#include <vector>


using namespace eeprom;

struct MockEEPromTaskClient {
    void send_eeprom_queue(const task::TaskMessage& m) {
        messages.push_back(m);
    }
    std::vector<task::TaskMessage> messages{};
};


struct MockListener {
    void on_read(const serial_number::SerialNumberType&) {}
};


SCENARIO("Writing serial number") {
    auto queue_client =MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto subject =
        serial_number::SerialNumberAccessor{queue_client, read_listener};

    GIVEN("A serial number to write") {
        auto data = serial_number::SerialNumberType{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        WHEN("the writing serial number") {
            subject.write(data);

            THEN("there are two eeprom writes") {
                REQUIRE(queue_client.messages.size() == 2);

                auto write_message = std::get<message::WriteEepromMessage>(queue_client.messages[0]);
                REQUIRE(write_message.memory_address==0);
                REQUIRE(write_message.length==8);
                REQUIRE(write_message.data==types::EepromData{1, 2, 3, 4, 5, 6, 7, 8});

                write_message = std::get<message::WriteEepromMessage>(queue_client.messages[1]);
                REQUIRE(write_message.memory_address==8);
                REQUIRE(write_message.length==4);
                REQUIRE(write_message.data==types::EepromData{9, 10, 11, 12});
            }
        }
    }

    GIVEN("A request to read the serial number") {
        WHEN("reading the serial number") {
            subject.start_read();

            THEN("there are two eeprom reads") {
                REQUIRE(queue_client.messages.size() == 2);

                auto write_message = std::get<message::ReadEepromMessage>(queue_client.messages[0]);
                REQUIRE(write_message.memory_address==0);
                REQUIRE(write_message.length==8);
                REQUIRE(write_message.callback_param == &subject);

                write_message = std::get<message::ReadEepromMessage>(queue_client.messages[1]);
                REQUIRE(write_message.memory_address==8);
                REQUIRE(write_message.length==4);
                REQUIRE(write_message.callback_param == &subject);
            }
        }
    }

}
