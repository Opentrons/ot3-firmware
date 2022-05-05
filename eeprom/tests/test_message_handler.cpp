#include "catch2/catch.hpp"
#include "eeprom/core/message_handler.hpp"
#include "eeprom/core/task.hpp"
#include "common/tests/mock_message_writer.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "can/core/can_writer_task.hpp"


SCENARIO("Sending messages to Eeprom CAN message handler") {
    auto can_write_queue = test_mocks::MockMessageQueue<message_writer_task::TaskMessage>{};
    auto eeprom_queue = test_mocks::MockMessageQueue<eeprom::task::TaskMessage>{};
    auto eeprom_task_client = mock_client::QueueClient{};
    auto can_writer_client = mock_message_writer::MockMessageWriter<test_mocks::MockMessageQueue>{};

    eeprom_task_client.eeprom_queue = &eeprom_queue;
    can_writer_client.set_queue(&can_write_queue);

    auto subject = eeprom::message_handler::EEPromHandler{eeprom_task_client, can_writer_client};

    GIVEN("A WriteToEEPromRequest message") {
        auto data = eeprom::types::EepromData{1, 2, 3, 4};

        auto msg = eeprom::message_handler::MessageType{can_messages::WriteToEEPromRequest{.address=22, .data_length=data.size(), .data=data}};
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
                REQUIRE(eeprom_message.memory_address == std::get<can_messages::WriteToEEPromRequest>(msg).address);
            }
            THEN("the size is set") {
                REQUIRE(eeprom_message.length == std::get<can_messages::WriteToEEPromRequest>(msg).data_length);
            }
            THEN("the data is set") {
                REQUIRE(eeprom_message.data == std::get<can_messages::WriteToEEPromRequest>(msg).data);
            }
        }
    }
    GIVEN("A ReadFromEEPromRequest message") {
        auto msg = eeprom::message_handler::MessageType{can_messages::ReadFromEEPromRequest{.address=5, .data_length=8}};

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
                REQUIRE(eeprom_message.memory_address == std::get<can_messages::ReadFromEEPromRequest>(msg).address);
            }
            THEN("the size is set") {
                REQUIRE(eeprom_message.length == std::get<can_messages::ReadFromEEPromRequest>(msg).data_length);
            }
            THEN("the callback is set") {
                REQUIRE(eeprom_message.callback == decltype(subject)::callback);
            }
            THEN("the param is set") {
                REQUIRE(eeprom_message.callback_param == &subject);
            }
        }
    }
}
