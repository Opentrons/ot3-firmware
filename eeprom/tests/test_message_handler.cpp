#include "can/core/can_writer_task.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "eeprom/core/message_handler.hpp"
#include "eeprom/core/task.hpp"

SCENARIO("Sending messages to Eeprom CAN message handler") {
    auto can_write_queue =
        test_mocks::MockMessageQueue<message_writer_task::TaskMessage>{};
    auto eeprom_queue =
        test_mocks::MockMessageQueue<eeprom::task::TaskMessage>{};
    auto queue_client = mock_client::QueueClient{};

    queue_client.eeprom_queue = &eeprom_queue;
    queue_client.set_queue(&can_write_queue);

    auto subject =
        eeprom::message_handler::EEPromHandler{queue_client, queue_client};

    GIVEN("A WriteToEEPromRequest message") {
        auto data = eeprom::types::EepromData{1, 2, 3, 4};

        auto can_msg = can_messages::WriteToEEPromRequest{
            .address = 22, .data_length = data.size(), .data = data};
        auto msg = eeprom::message_handler::MessageType{can_msg};
        WHEN("the message is received") {
            subject.handle(msg);

            THEN("the message is written to the eeprom client") {
                REQUIRE(eeprom_queue.get_size() == 1);
            }

            auto queue_message = eeprom::task::TaskMessage{};
            eeprom_queue.try_read(&queue_message);
            auto eeprom_message =
                std::get<eeprom::message::WriteEepromMessage>(queue_message);

            THEN("the address is set") {
                REQUIRE(eeprom_message.memory_address == can_msg.address);
            }
            THEN("the size is set") {
                REQUIRE(eeprom_message.length == can_msg.data_length);
            }
            THEN("the data is set") {
                REQUIRE(eeprom_message.data == can_msg.data);
            }
        }
    }
    GIVEN("A ReadFromEEPromRequest message") {
        auto can_msg =
            can_messages::ReadFromEEPromRequest{.address = 5, .data_length = 8};
        auto msg = eeprom::message_handler::MessageType{can_msg};

        WHEN("the message is received") {
            subject.handle(msg);

            THEN("the message is written to the eeprom client") {
                REQUIRE(eeprom_queue.get_size() == 1);
            }

            auto queue_message = eeprom::task::TaskMessage{};
            eeprom_queue.try_read(&queue_message);
            auto eeprom_message =
                std::get<eeprom::message::ReadEepromMessage>(queue_message);

            THEN("the address is set") {
                REQUIRE(eeprom_message.memory_address == can_msg.address);
            }
            THEN("the size is set") {
                REQUIRE(eeprom_message.length == can_msg.data_length);
            }
            THEN("the param is set to the address of the handler") {
                REQUIRE(eeprom_message.callback_param == &subject);
            }
        }
    }

    GIVEN("A complete read message transaction") {
        auto can_msg =
            can_messages::ReadFromEEPromRequest{.address = 5, .data_length = 8};
        auto msg = eeprom::message_handler::MessageType{can_msg};
        subject.handle(msg);

        auto queue_message = eeprom::task::TaskMessage{};
        eeprom_queue.try_read(&queue_message);
        auto eeprom_message =
            std::get<eeprom::message::ReadEepromMessage>(queue_message);

        WHEN("the message is responded to") {
            auto response = eeprom::message::EepromMessage{
                .memory_address = can_msg.address,
                .length = can_msg.data_length,
                .data{1, 2, 3, 4, 5, 6, 7, 8}};

            eeprom_message.callback(response, eeprom_message.callback_param);

            THEN("a response message is written to the can writer") {
                REQUIRE(can_write_queue.get_size() == 1);
            }

            auto can_queue_message = message_writer_task::TaskMessage{};
            can_write_queue.try_read(&can_queue_message);
            auto can_message = std::get<can_messages::ReadFromEEPromResponse>(
                can_queue_message.message);

            THEN(
                "the response message is populated with data in eeprom "
                "response") {
                REQUIRE(can_message.address == response.memory_address);
                REQUIRE(can_message.data_length == response.length);
                REQUIRE(can_message.data == response.data);
            }
        }
    }
}
