#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/serial_number.hpp"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "eeprom/tests/mock_eeprom_task_client.hpp"

using namespace eeprom;

SCENARIO("Writing serial number") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto sn_buffer = serial_number::SerialNumberType{};
    auto sn_data = serial_number::SerialNumberType{0x00, 0x01, 0x00, 0x1f, '2',
                                                   '0',  '2',  '0',  '1',  '9',
                                                   '0',  '7',  'A',  '0',  '3'};
    auto subject = serial_number::SerialNumberAccessor{
        queue_client, read_listener, sn_buffer};

    GIVEN("A serial number to write") {
        WHEN("the writing serial number") {
            subject.write(sn_data);

            THEN("there is an eeprom write") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::serial_number_length / types::max_data_length) +
                        (addresses::serial_number_length %
                             types::max_data_length !=
                         0));
                message::WriteEepromMessage write_message;
                types::data_length expected_bytes;
                for (size_t i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((sn_data.size() - (i * types::max_data_length)) >
                                 types::max_data_length
                             ? types::max_data_length
                             : sn_data.size() % types::max_data_length);

                    write_message = std::get<message::WriteEepromMessage>(
                        queue_client.messages[i]);
                    REQUIRE(write_message.memory_address ==
                            addresses::serial_number_address_begin +
                                (i * types::max_data_length));

                    REQUIRE(write_message.length == expected_bytes);
                    REQUIRE(std::equal(write_message.data.begin(),
                                       &(write_message.data[expected_bytes]),
                                       &(sn_data[i * types::max_data_length])));
                }
            }
        }
    }
}

SCENARIO("Reading serial number") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto sn_buffer = serial_number::SerialNumberType{};
    auto sn_data = serial_number::SerialNumberType{'P', '1', 'K', 'S', 'V', '2',
                                                   '0', '2', '0', '1', '9', '0',
                                                   '7', '2', '4', '3', '0'};

    auto subject = serial_number::SerialNumberAccessor{
        queue_client, read_listener, sn_buffer};
    GIVEN("A request to read the serial number") {
        WHEN("reading the serial number") {
            subject.start_read();

            THEN("there is an eeprom read") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::serial_number_length / types::max_data_length) +
                        (addresses::serial_number_length %
                             types::max_data_length !=
                         0));

                types::data_length expected_bytes;
                message::ReadEepromMessage read_message;
                for (size_t i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::serial_number_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::serial_number_length %
                                   types::max_data_length);

                    read_message = std::get<message::ReadEepromMessage>(
                        queue_client.messages[i]);

                    REQUIRE(read_message.memory_address ==
                            addresses::serial_number_address_begin +
                                (i * types::max_data_length));
                    REQUIRE(read_message.length == expected_bytes);
                    REQUIRE(read_message.callback_param == &subject);
                }
            }
        }
    }

    GIVEN("A request to read the serial number") {
        subject.start_read();

        WHEN("the read completes") {
            message::ReadEepromMessage read_message;
            types::data_length num_bytes;

            auto data = types::EepromData{};
            auto expected_num_read_response =
                (addresses::serial_number_length / types::max_data_length) +
                (addresses::serial_number_length % types::max_data_length != 0);

            for (auto i = 0; i < expected_num_read_response; i++) {
                num_bytes =
                    ((addresses::serial_number_length -
                      (i * types::max_data_length)) > types::max_data_length
                         ? types::max_data_length
                         : addresses::serial_number_length %
                               types::max_data_length);
                read_message = std::get<message::ReadEepromMessage>(
                    queue_client.messages[i]);
                std::copy_n(sn_data.begin() + (i * types::max_data_length),
                            num_bytes, data.begin());
                read_message.callback(
                    message::EepromMessage{
                        .memory_address = read_message.memory_address,
                        .length = num_bytes,
                        .data = data},
                    read_message.callback_param);
            }
            THEN("then the listener is called once") {
                REQUIRE(read_listener.call_count == 1);
                REQUIRE(sn_buffer == sn_data);
            }
        }
    }
}
