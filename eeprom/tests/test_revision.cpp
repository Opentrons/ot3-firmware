#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/revision.hpp"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "eeprom/tests/mock_eeprom_task_client.hpp"

using namespace eeprom;

SCENARIO("Writing revision") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto rev_buffer = revision::RevisionType{};
    auto subject =
        revision::RevisionAccessor{queue_client, read_listener, rev_buffer};

    GIVEN("A revision to write") {
        auto rev_data = revision::RevisionType{0, 1, 2, 3};

        WHEN("the writing revision") {
            subject.write(rev_data);

            THEN("there is an eeprom write") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::revision_length / types::max_data_length) +
                        (addresses::revision_length % types::max_data_length !=
                         0));
                message::WriteEepromMessage write_message;
                types::data_length expected_bytes;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::revision_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::revision_length %
                                   types::max_data_length);

                    write_message = std::get<message::WriteEepromMessage>(
                        queue_client.messages[i]);
                    REQUIRE(write_message.memory_address ==
                            addresses::revision_address_begin +
                                (i * types::max_data_length));

                    REQUIRE(write_message.length == expected_bytes);
                    REQUIRE(
                        std::equal(write_message.data.begin(),
                                   &(write_message.data[expected_bytes]),
                                   &(rev_data[i * types::max_data_length])));
                }
            }
        }
    }
}

SCENARIO("Reading revision") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto rev_buffer = revision::RevisionType{};
    auto subject =
        revision::RevisionAccessor{queue_client, read_listener, rev_buffer};

    GIVEN("A request to read the revision") {
        WHEN("reading the revision") {
            subject.start_read();

            THEN("there is an eeprom read") {
                REQUIRE(
                    queue_client.messages.size() ==
                    (addresses::revision_length / types::max_data_length) +
                        (addresses::revision_length % types::max_data_length !=
                         0));

                types::data_length expected_bytes;
                message::ReadEepromMessage read_message;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::revision_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::revision_length %
                                   types::max_data_length);

                    read_message = std::get<message::ReadEepromMessage>(
                        queue_client.messages[i]);

                    REQUIRE(read_message.memory_address ==
                            addresses::revision_address_begin +
                                (i * types::max_data_length));
                    REQUIRE(read_message.length == expected_bytes);
                    REQUIRE(read_message.callback_param == &subject);
                }
            }
        }
    }

    GIVEN("A request to read the revision") {
        subject.start_read();

        WHEN("the read completes") {
            rev_buffer.fill(0x00);
            auto rev_data = revision::RevisionType{4, 5, 6, 7};

            message::ReadEepromMessage read_message;
            types::data_length num_bytes;

            auto data = types::EepromData{};
            auto expected_num_read_response =
                (addresses::revision_length / types::max_data_length) +
                (addresses::revision_length % types::max_data_length != 0);

            for (auto i = 0; i < expected_num_read_response; i++) {
                num_bytes =
                    ((addresses::revision_length -
                      (i * types::max_data_length)) > types::max_data_length
                         ? types::max_data_length
                         : addresses::revision_length % types::max_data_length);
                read_message = std::get<message::ReadEepromMessage>(
                    queue_client.messages[i]);

                std::copy_n(rev_data.begin() + (i * types::max_data_length),
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
                REQUIRE(rev_buffer == rev_data);
            }
        }
    }
}
