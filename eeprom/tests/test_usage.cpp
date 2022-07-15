#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/accessor.hpp"
#include "eeprom/core/usage.hpp"
#include "eeprom/tests/mock_eeprom_task_client.hpp"

using namespace eeprom;

struct U_MockListener : accessor::ReadListener<usage::UsageType> {
    void on_read(const usage::UsageType& usage) {
        this->usage = usage;
        call_count++;
    }
    usage::UsageType usage{};
    int call_count{0};
};

SCENARIO("Writing usage") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = U_MockListener{};
    auto subject = usage::UsageAccessor{queue_client, read_listener};

    GIVEN("A usage to write") {
        auto data = usage::UsageType{1, 2, 3, 4};

        WHEN("the writing usage") {
            subject.write(data);

            THEN("there is an eeprom write") {
                REQUIRE(queue_client.messages.size() ==
                        (addresses::usage_length / types::max_data_length) +
                            (addresses::usage_length % types::max_data_length !=
                             0));
                message::WriteEepromMessage write_message;
                types::data_length expected_bytes;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::usage_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::usage_length %
                                   types::max_data_length);

                    write_message = std::get<message::WriteEepromMessage>(
                        queue_client.messages[i]);
                    REQUIRE(write_message.memory_address ==
                            addresses::usage_address_begin +
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

SCENARIO("Reading usage") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = U_MockListener{};
    auto subject = usage::UsageAccessor{queue_client, read_listener};

    GIVEN("A request to read the usage") {
        WHEN("reading the usage") {
            subject.start_read();

            THEN("there is an eeprom read") {
                REQUIRE(queue_client.messages.size() ==
                        (addresses::usage_length / types::max_data_length) +
                            (addresses::usage_length % types::max_data_length !=
                             0));

                types::data_length expected_bytes;
                message::ReadEepromMessage read_message;
                for (ulong i = 0; i < queue_client.messages.size(); i++) {
                    expected_bytes =
                        ((addresses::usage_length -
                          (i * types::max_data_length)) > types::max_data_length
                             ? types::max_data_length
                             : addresses::usage_length %
                                   types::max_data_length);

                    read_message = std::get<message::ReadEepromMessage>(
                        queue_client.messages[i]);

                    REQUIRE(read_message.memory_address ==
                            addresses::usage_address_begin +
                                (i * types::max_data_length));
                    REQUIRE(read_message.length == expected_bytes);
                    REQUIRE(read_message.callback_param == &subject);
                }
            }
        }
    }

    GIVEN("A request to read the usage") {
        subject.start_read();

        WHEN("the read completes") {
            auto usage = usage::UsageType{2, 4, 8, 16};

            message::ReadEepromMessage read_message;
            types::data_length num_bytes;

            auto data = types::EepromData{};
            auto expected_num_read_response =
                (addresses::usage_length / types::max_data_length) +
                (addresses::usage_length % types::max_data_length != 0);

            for (auto i = 0; i < expected_num_read_response; i++) {
                num_bytes =
                    ((addresses::usage_length - (i * types::max_data_length)) >
                             types::max_data_length
                         ? types::max_data_length
                         : addresses::usage_length % types::max_data_length);
                read_message = std::get<message::ReadEepromMessage>(
                    queue_client.messages[i]);

                std::copy_n(usage.cbegin() + (i * types::max_data_length),
                            num_bytes, data.begin());
                read_message.callback(
                    {.memory_address = read_message.memory_address,
                     .length = num_bytes,
                     .data = data},
                    read_message.callback_param);
            }
            THEN("then the listener is called once") {
                REQUIRE(read_listener.call_count == 1);
                REQUIRE(read_listener.usage == usage);
            }
        }
    }
}
// TODO (rtc 07-15-22) make this size independent for Usage type
// doubt that we'll ever make it size(UsageType) > types::max_data_length
// but this will fail if we do
SCENARIO("Increase the Usage") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = U_MockListener{};
    auto subject = usage::UsageAccessor{queue_client, read_listener};

    auto time_to_add = usage::UsageType{0x1, 0x1, 0x0, 0x0};
    auto old_time = usage::UsageType{0x0, 0x0, 0x1, 0x1};
    auto expected_new_usage = usage::UsageType{0x1, 0x1, 0x1, 0x1};

    GIVEN("A request to increase usage") {
        subject.increase_usage(time_to_add);
        WHEN("increase request sent") {
            // simulate the read by populating the read resposne with old_time
            message::ReadEepromMessage read_message;
            types::data_length num_bytes = sizeof(usage::UsageType);
            auto data = types::EepromData{};
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            std::copy_n(old_time.begin(), num_bytes, data.begin());
            read_message.callback(
                {.memory_address = read_message.memory_address,
                 .length = num_bytes,
                 .data = data},
                read_message.callback_param);
            THEN("There is a write with correct values") {
                // should have queued a read and a write
                REQUIRE(queue_client.messages.size() == 2);
                auto write_message = std::get<message::WriteEepromMessage>(
                    queue_client.messages[1]);
                REQUIRE(std::equal(
                    write_message.data.begin(),
                    write_message.data.begin() + sizeof(usage::UsageType),
                    expected_new_usage.begin()));
            }
        }
    }
}
