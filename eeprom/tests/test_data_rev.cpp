#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/accessor.hpp"
#include "eeprom/core/data_rev.hpp"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "eeprom/tests/mock_eeprom_task_client.hpp"

using namespace eeprom;


SCENARIO("Writing data revision") {
    auto queue_client = MockEEPromTaskClient{};
    auto subject = data_revision::DataRevAccessor{queue_client, read_listener};
    auto read_listener = MockListener{};

    GIVEN("A data revision to write") {
        auto data = data_revision::DataRevisionType{1, 2};

        WHEN("the writing data revision") {
            subject.write(data);

            THEN("there is an eeprom write") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::data_revision_length / types::max_data_length) +
                        (addresses::data_revision_length %
                             types::max_data_length !=
                         0));
                message::WriteEepromMessage write_message;
                types::data_length expected_bytes;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::data_revision_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::data_revision_length %
                                   types::max_data_length);

                    write_message = std::get<message::WriteEepromMessage>(
                        queue_client.messages[i]);
                    REQUIRE(write_message.memory_address ==
                            addresses::data_revision_address_begin +
                                (i * types::max_data_length));

                    REQUIRE(write_message.length == expected_bytes);
                    REQUIRE(std::equal(write_message.data.begin(),
                                       &(write_message.data[expected_bytes]),
                                       &(data[i * types::max_data_length])));
                }
            }
        }
    }
}

SCENARIO("Reading data revision") {
    auto queue_client = MockEEPromTaskClient{};
    auto subject = data_revision::DataRevAccessor{queue_client, read_listener};
    auto read_listener = MockListener{};

    GIVEN("A request to read the data revision") {
        WHEN("reading the data revision") {
            subject.start_read();

            THEN("there is an eeprom read") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::data_revision_length / types::max_data_length) +
                        (addresses::data_revision_length %
                             types::max_data_length !=
                         0));

                types::data_length expected_bytes;
                message::ReadEepromMessage read_message;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::data_revision_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::data_revision_length %
                                   types::max_data_length);

                    read_message = std::get<message::ReadEepromMessage>(
                        queue_client.messages[i]);

                    REQUIRE(read_message.memory_address ==
                            addresses::data_revision_address_begin +
                                (i * types::max_data_length));
                    REQUIRE(read_message.length == expected_bytes);
                    REQUIRE(read_message.callback_param == &subject);
                }
            }
        }
    }

    GIVEN("A request to read the data revision") {
        subject.start_read();

        WHEN("the read completes") {
            auto data_rev = data_revision::DataRevisionType{2, 4};

            message::ReadEepromMessage read_message;
            types::data_length num_bytes;

            auto data = types::EepromData{};
            auto expected_num_read_response =
                (addresses::data_revision_length / types::max_data_length) +
                (addresses::data_revision_length % types::max_data_length != 0);

            for (auto i = 0; i < expected_num_read_response; i++) {
                num_bytes =
                    ((addresses::data_revision_length -
                      (i * types::max_data_length)) > types::max_data_length
                         ? types::max_data_length
                         : addresses::data_revision_length %
                               types::max_data_length);
                read_message = std::get<message::ReadEepromMessage>(
                    queue_client.messages[i]);

                std::copy_n(data_rev.cbegin() + (i * types::max_data_length),
                            num_bytes, data.begin());
                read_message.callback(
                    {.memory_address = read_message.memory_address,
                     .length = num_bytes,
                     .data = data},
                    read_message.callback_param);
            }
            THEN("then the listener is called once") {
                REQUIRE(read_listener.call_count == 1);
                REQUIRE(read_listener.data_rev == data_rev);
            }
        }
    }
}
