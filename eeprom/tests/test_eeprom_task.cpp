#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

class MockHardwareIface : public eeprom::hardware_iface::EEPromHardwareIface {
    using eeprom::hardware_iface::EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

SCENARIO("Sending messages to Eeprom task") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface = MockHardwareIface{};

    auto eeprom = eeprom::task::EEPromMessageHandler{writer, response_queue,
                                                     hardware_iface};
    GIVEN("A write message") {
        auto data = eeprom::types::EepromData{1, 2, 3, 4};
        eeprom::types::address address = 12;
        eeprom::types::data_length data_length = 4;
        auto write_msg = eeprom::task::TaskMessage(
            eeprom::message::WriteEepromMessage{.memory_address = address,
                                                .length = data_length,
                                                .data = data});
        WHEN("the message is sent") {
            eeprom.handle_message(write_msg);
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message =
                    std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address == 0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read == 0);
                REQUIRE(
                    transact_message.transaction.bytes_to_write ==
                    static_cast<std::size_t>(
                        data_length + hardware_iface.get_eeprom_addr_bytes()));
                REQUIRE(transact_message.transaction.write_buffer[0] ==
                        address);
                REQUIRE(transact_message.transaction.write_buffer[1] ==
                        data[0]);
                REQUIRE(transact_message.transaction.write_buffer[2] ==
                        data[1]);
                REQUIRE(transact_message.transaction.write_buffer[3] ==
                        data[2]);
                REQUIRE(transact_message.transaction.write_buffer[4] ==
                        data[3]);
                REQUIRE(transact_message.id.token == eeprom.WRITE_TOKEN);
            }
        }
    }
    GIVEN("A write message with zero length") {
        auto data = eeprom::types::EepromData{};
        eeprom::types::address address = 14;
        eeprom::types::data_length data_length = 0;
        auto write_msg = eeprom::task::TaskMessage(
            eeprom::message::WriteEepromMessage{.memory_address = address,
                                                .length = data_length,
                                                .data = data});
        WHEN("the message is sent") {
            eeprom.handle_message(write_msg);
            THEN("the i2c queue is not populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 0);
            }
        }
    }
    GIVEN("A read message") {
        eeprom::types::address address = 14;
        eeprom::types::data_length data_length = 5;
        auto read_msg =
            eeprom::task::TaskMessage(eeprom::message::ReadEepromMessage{
                .memory_address = address, .length = data_length});
        eeprom.handle_message(read_msg);
        WHEN("the message is sent") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message =
                    std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address == 0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read ==
                        data_length);
                REQUIRE(transact_message.transaction.bytes_to_write ==
                        hardware_iface.get_eeprom_addr_bytes());
                REQUIRE(transact_message.transaction.write_buffer[0] ==
                        address);
                REQUIRE(transact_message.id.token == 0);
            }
        }
    }
    GIVEN("A read message with zero length") {
        eeprom::types::address address = 14;
        eeprom::types::data_length data_length = 0;
        auto read_msg =
            eeprom::task::TaskMessage(eeprom::message::ReadEepromMessage{
                .memory_address = address, .length = data_length});
        eeprom.handle_message(read_msg);
        WHEN("the message is sent") {
            THEN("the i2c queue is not populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 0);
            }
        }
    }
}
SCENARIO("Sending messages to 16 bit address Eeprom task") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    auto hardware_iface_16 =
        MockHardwareIface(eeprom::hardware_iface::EEPROM_ADDR_16_BIT);

    auto eeprom_16 = eeprom::task::EEPromMessageHandler{writer, response_queue,
                                                        hardware_iface_16};
    GIVEN("A 16 bit write message") {
        auto data = eeprom::types::EepromData{1, 2, 3, 4};
        eeprom::types::address address = 0xabcd;
        eeprom::types::data_length data_length = 4;
        auto write_msg = eeprom::task::TaskMessage(
            eeprom::message::WriteEepromMessage{.memory_address = address,
                                                .length = data_length,
                                                .data = data});
        WHEN("the message is sent") {
            eeprom_16.handle_message(write_msg);
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message =
                    std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address == 0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read == 0);
                REQUIRE(transact_message.transaction.bytes_to_write ==
                        static_cast<std::size_t>(
                            data_length +
                            hardware_iface_16.get_eeprom_addr_bytes()));
                REQUIRE(transact_message.transaction.write_buffer[0] ==
                        ((address >> 8) & 0xff));
                REQUIRE(transact_message.transaction.write_buffer[1] ==
                        (address & 0xff));
                REQUIRE(transact_message.transaction.write_buffer[2] ==
                        data[0]);
                REQUIRE(transact_message.transaction.write_buffer[3] ==
                        data[1]);
                REQUIRE(transact_message.transaction.write_buffer[4] ==
                        data[2]);
                REQUIRE(transact_message.transaction.write_buffer[5] ==
                        data[3]);
                REQUIRE(transact_message.id.token == eeprom_16.WRITE_TOKEN);
            }
        }
    }
    GIVEN("A 16 bit read message") {
        eeprom::types::address address = 0xabcd;
        eeprom::types::data_length data_length = 5;
        auto read_msg =
            eeprom::task::TaskMessage(eeprom::message::ReadEepromMessage{
                .memory_address = address, .length = data_length});
        eeprom_16.handle_message(read_msg);
        WHEN("the message is sent") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);

                auto i2c_message = i2c::writer::TaskMessage{};
                i2c_queue.try_read(&i2c_message);

                auto transact_message =
                    std::get<i2c::messages::Transact>(i2c_message);
                REQUIRE(transact_message.transaction.address == 0xA0);
                REQUIRE(transact_message.transaction.bytes_to_read ==
                        data_length);
                REQUIRE(transact_message.transaction.bytes_to_write ==
                        hardware_iface_16.get_eeprom_addr_bytes());
                REQUIRE(transact_message.transaction.write_buffer[0] ==
                        ((address >> 8) & 0xff));
                REQUIRE(transact_message.transaction.write_buffer[1] ==
                        (address & 0xff));
                REQUIRE(transact_message.id.token == 0);
            }
        }
    }
}

struct ReadResponseHandler {
    static void callback(const eeprom::message::EepromMessage& msg,
                         void* param) {
        reinterpret_cast<ReadResponseHandler*>(param)->_callback(msg);
    }
    void _callback(const eeprom::message::EepromMessage& msg) { message = msg; }
    eeprom::message::EepromMessage message;
};

SCENARIO("Transaction response handling.") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface = MockHardwareIface{};

    auto eeprom = eeprom::task::EEPromMessageHandler{writer, response_queue,
                                                     hardware_iface};

    GIVEN("A read request") {
        eeprom::types::address address = 14;
        eeprom::types::data_length data_length = 5;
        auto read_response_handler = ReadResponseHandler{};
        auto read_msg =
            eeprom::task::TaskMessage(eeprom::message::ReadEepromMessage{
                .memory_address = address,
                .length = data_length,
                .callback = ReadResponseHandler::callback,
                .callback_param = &read_response_handler});
        eeprom.handle_message(read_msg);

        WHEN("a transaction response is sent") {
            auto transaction_response =
                eeprom::task::TaskMessage(i2c::messages::TransactionResponse{
                    .id = i2c::messages::TransactionIdentifier{.token = 0},
                    .bytes_read = data_length,
                    .read_buffer =
                        i2c::messages::MaxMessageBuffer{1, 2, 3, 4, 5}});

            eeprom.handle_message(transaction_response);
            THEN("the callback is called") {
                REQUIRE(read_response_handler.message ==
                        eeprom::message::EepromMessage{
                            .memory_address = address,
                            .length = data_length,
                            .data = eeprom::types::EepromData{1, 2, 3, 4, 5}});
            }
        }
    }

    GIVEN("A write request") {
        auto data = eeprom::types::EepromData{1, 2, 3};
        eeprom::types::address address = 14;
        eeprom::types::data_length data_length = 3;
        auto write_msg = eeprom::task::TaskMessage(
            eeprom::message::WriteEepromMessage{.memory_address = address,
                                                .length = data_length,
                                                .data = data});
        eeprom.handle_message(write_msg);

        WHEN("a transaction response is sent") {
            auto transaction_response =
                eeprom::task::TaskMessage(i2c::messages::TransactionResponse{
                    .id =
                        i2c::messages::TransactionIdentifier{
                            .token = static_cast<uint32_t>(-1)},
                    .bytes_read = 0,
                    .read_buffer = i2c::messages::MaxMessageBuffer{}});

            eeprom.handle_message(transaction_response);
            THEN("the write protect pin is disabled then enabled") {
                REQUIRE(hardware_iface.set_calls ==
                        std::vector<bool>{false, true});
            }
        }
    }
}
