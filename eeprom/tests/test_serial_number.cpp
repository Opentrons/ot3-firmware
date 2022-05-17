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
    void on_read(const serial_number::SerialNumberType& sn) {this->sn = sn; call_count++;}
    serial_number::SerialNumberType sn{};
    int call_count{0};
};


SCENARIO("Writing serial number") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto subject =
        serial_number::SerialNumberAccessor{queue_client, read_listener};

    GIVEN("A serial number to write") {
        auto data = serial_number::SerialNumberType{1, 2, 3, 4,  5,  6,
                                                    7, 8, 9, 10, 11, 12};

        WHEN("the writing serial number") {
            subject.write(data);

            THEN("there are two eeprom writes") {
                REQUIRE(queue_client.messages.size() == 2);

                auto write_message = std::get<message::WriteEepromMessage>(
                    queue_client.messages[0]);
                REQUIRE(write_message.memory_address == 0);
                REQUIRE(write_message.length == 8);
                REQUIRE(write_message.data ==
                        types::EepromData{1, 2, 3, 4, 5, 6, 7, 8});

                write_message = std::get<message::WriteEepromMessage>(
                    queue_client.messages[1]);
                REQUIRE(write_message.memory_address == 8);
                REQUIRE(write_message.length == 4);
                REQUIRE(write_message.data == types::EepromData{9, 10, 11, 12});
            }
        }
    }
}


SCENARIO("Reading serial number") {
    auto queue_client =MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto subject =
        serial_number::SerialNumberAccessor{queue_client, read_listener};

    GIVEN("A request to read the serial number") {
        WHEN("reading the serial number") {
            subject.start_read();

            THEN("there are two eeprom reads") {
                REQUIRE(queue_client.messages.size() == 2);

                auto read_message = std::get<message::ReadEepromMessage>(queue_client.messages[0]);
                REQUIRE(read_message.memory_address==0);
                REQUIRE(read_message.length==8);
                REQUIRE(read_message.callback_param == &subject);

                read_message = std::get<message::ReadEepromMessage>(queue_client.messages[1]);
                REQUIRE(read_message.memory_address==8);
                REQUIRE(read_message.length==4);
                REQUIRE(read_message.callback_param == &subject);
            }
        }
    }

    GIVEN("A request to read the serial number") {
        subject.start_read();

        WHEN("the read completes") {
            auto sn = serial_number::SerialNumberType{12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

            for (auto m = queue_client.messages.cbegin(); m < queue_client.messages.cend(); m++) {
                auto read_message = std::get<message::ReadEepromMessage>(*m);
                auto data = types::EepromData{};
                std::copy_n(sn.cbegin() + read_message.memory_address, read_message.length, data.begin());

                read_message.callback({
                    .memory_address=read_message.memory_address,
                        .length=read_message.length,
                    .data=data}, read_message.callback_param);
            }

            THEN("then the listener is called once") {
                REQUIRE(read_listener.call_count == 1);
                REQUIRE(read_listener.sn == sn);
            }
        }
    }
}
