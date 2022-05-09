#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

SCENARIO("read and write pipette serial numbers") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<eeprom::task::TaskMessage> eeprom_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    i2c::writer::TaskMessage empty_msg{};
    auto queue_client = mock_client::QueueClient{.eeprom_queue = &eeprom_queue};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);

    auto eeprom = eeprom::task::EEPromMessageHandler{writer, queue_client,
                                                     response_queue};

    GIVEN("write message from CAN") {
        uint16_t serial_number = 0x2;
        auto write_msg = eeprom::task::TaskMessage(
            can_messages::WriteToEEPromRequest({}, serial_number));
        eeprom.handle_message(write_msg);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a write command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
        }
    }
    GIVEN("a read message from CAN") {
        auto read_msg =
            eeprom::task::TaskMessage(can_messages::ReadFromEEPromRequest());
        eeprom.handle_message(read_msg);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
        }
    }
}
