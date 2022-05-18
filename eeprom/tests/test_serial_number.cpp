#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/serial_number.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    void send_eeprom_queue(const task::TaskMessage& m) {
        messages.push_back(m);
    }
    std::vector<task::TaskMessage> messages{};
};

struct MockListener : serial_number::ReadListener {
    void on_read(const serial_number::SerialNumberType& sn) {
        this->sn = sn;
        call_count++;
    }
    serial_number::SerialNumberType sn{};
    int call_count{0};
};

SCENARIO("Writing serial number") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto subject =
        serial_number::SerialNumberAccessor{queue_client, read_listener};

    GIVEN("A serial number to write") {
        auto data = serial_number::SerialNumberType{1, 2, 3, 4, 5, 6, 7, 8};

        WHEN("the writing serial number") {
            subject.write(data);

            THEN("there is an eeprom write") {
                REQUIRE(queue_client.messages.size() == 1);

                auto write_message = std::get<message::WriteEepromMessage>(
                    queue_client.messages[0]);
                REQUIRE(write_message.memory_address ==
                        addresses::serial_number_address_begin);
                REQUIRE(write_message.length == types::max_data_length);
                REQUIRE(write_message.data ==
                        types::EepromData{1, 2, 3, 4, 5, 6, 7, 8});
            }
        }
    }
}

SCENARIO("Reading serial number") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto subject =
        serial_number::SerialNumberAccessor{queue_client, read_listener};

    GIVEN("A request to read the serial number") {
        WHEN("reading the serial number") {
            subject.start_read();

            THEN("there is an eeprom read") {
                REQUIRE(queue_client.messages.size() == 1);

                auto read_message = std::get<message::ReadEepromMessage>(
                    queue_client.messages[0]);
                REQUIRE(read_message.memory_address ==
                        addresses::serial_number_address_begin);
                REQUIRE(read_message.length == types::max_data_length);
                REQUIRE(read_message.callback_param == &subject);
            }
        }
    }

    GIVEN("A request to read the serial number") {
        subject.start_read();

        WHEN("the read completes") {
            auto sn = serial_number::SerialNumberType{8, 7, 6, 5, 4, 3, 2, 1};

            for (auto m = queue_client.messages.cbegin();
                 m < queue_client.messages.cend(); m++) {
                auto read_message = std::get<message::ReadEepromMessage>(*m);
                auto data = types::EepromData{};
                std::copy_n(sn.cbegin() + read_message.memory_address,
                            read_message.length, data.begin());

                read_message.callback(
                    {.memory_address = read_message.memory_address,
                     .length = read_message.length,
                     .data = data},
                    read_message.callback_param);
            }

            THEN("then the listener is called once") {
                REQUIRE(read_listener.call_count == 1);
                REQUIRE(read_listener.sn == sn);
            }
        }
    }
}
