#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/types.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"


class TestEepromResponseHandler : public eeprom::message::EepromResponseHandler {

};


SCENARIO("Eeprom task interaction") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<eeprom::task::TaskMessage> eeprom_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    i2c::writer::TaskMessage empty_msg{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    auto eeprom = eeprom::task::EEPromMessageHandler{writer, response_queue};

    GIVEN("A write message") {
        auto data = eeprom::types::EepromData{1, 2, 3, 4};
        eeprom::types::address address=12;
        eeprom::types::data_length data_length = 4;
        auto write_msg = eeprom::task::TaskMessage(eeprom::message::WriteEepromMessage{.memory_address=address, .length=data_length, .data=data});
        WHEN("the message is sent") {
            eeprom.handle_message(write_msg);
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message = std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address==0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read==0);
                REQUIRE(transact_message.transaction.bytes_to_write==static_cast<std::size_t>(data_length + 1));
                REQUIRE(transact_message.transaction.write_buffer[0] == address);
                REQUIRE(transact_message.transaction.write_buffer[1] == data[0]);
                REQUIRE(transact_message.transaction.write_buffer[2] == data[1]);
                REQUIRE(transact_message.transaction.write_buffer[3] == data[2]);
                REQUIRE(transact_message.transaction.write_buffer[4] == data[3]);
            }
        }
    }
    GIVEN("A read message") {
        eeprom::types::address address=14;
        eeprom::types::data_length data_length = 5;
        auto read_msg = eeprom::task::TaskMessage(eeprom::message::ReadEepromMessage{.memory_address=address, .length=data_length});
        eeprom.handle_message(read_msg);
        WHEN("the message is sent") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message = std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address==0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read==data_length);
                REQUIRE(transact_message.transaction.bytes_to_write==1);
                REQUIRE(transact_message.transaction.write_buffer[0] == address);
                REQUIRE(transact_message.id.token == 0);
            }
        }
    }
}
