#include <cstring>
#include <vector>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "eeprom/tests/mock_eeprom_task_client.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

using namespace eeprom;

class MockHardwareIface : public hardware_iface::EEPromHardwareIface {
    using hardware_iface::EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

SCENARIO("initalizing a data accessor") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};

    GIVEN("A creating a data accessor entry") {
        auto dev_data_buffer = dev_data::DataBufferType<64>{};
        auto subject = dev_data::DevDataAccessor{queue_client, read_listener,
                                                 dev_data_buffer};
        auto data = types::EepromData{};
        data.fill(0x00);
        THEN("accessor attemps to read data tail") {
            REQUIRE(queue_client.messages.size() == 2);
            auto read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            REQUIRE(read_message.memory_address ==
                    addresses::lookup_table_tail_begin);
            REQUIRE(read_message.length == addresses::lookup_table_tail_length);
        }
        THEN("accessor correctly sets up lookup table tail in eeprom") {
            auto read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            // respond back with 0x00'd out data to simulate new chip
            read_message.callback(
                message::EepromMessage{
                    .memory_address = read_message.memory_address,
                    .length = read_message.length,
                    .data = data},
                read_message.callback_param);
            auto write_message =
                std::get<message::WriteEepromMessage>(queue_client.messages[2]);
            REQUIRE(write_message.memory_address ==
                    addresses::lookup_table_tail_begin);
            REQUIRE(write_message.length ==
                    addresses::lookup_table_tail_length);
            data = types::EepromData{0, addresses::lookup_table_tail_end};
            REQUIRE(
                std::equal(write_message.data.begin(),
                           write_message.data.begin() + write_message.length,
                           data.begin()));
        }
        THEN("accessor recieves config from task") {
            // make sure the second message is a config request
            REQUIRE(std::get_if<message::ConfigRequestMessage>(
                        &queue_client.messages[1]) != nullptr);
        }
    }
}
SCENARIO("creating a data table entry") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto dev_data_buffer = dev_data::DataBufferType<64>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface = MockHardwareIface{};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};
    GIVEN("data accessor correctly initializes") {
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);
        queue_client.messages.clear();
        THEN("accessor attemps to create data table entry") {
            subject.create_data_part(0x00, 0x04);
            // for the first one we just expect a write to the first segment of
            // the table and we expect a read to the table_tail value so it can
            // be incremented
            REQUIRE(queue_client.messages.size() == 2);
            auto write_message =
                std::get<message::WriteEepromMessage>(queue_client.messages[0]);
            REQUIRE(write_message.memory_address ==
                    addresses::data_address_begin);
            REQUIRE(write_message.length ==
                    static_cast<uint16_t>(
                        hardware_iface::EEPromAddressType::EEPROM_ADDR_8_BIT) *
                        2);
            auto read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[1]);
            REQUIRE(read_message.memory_address ==
                    addresses::lookup_table_tail_begin);
            REQUIRE(read_message.length == addresses::lookup_table_tail_length);
        }
    }
}
SCENARIO("creating a data table on 16 bit addresss entry") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto dev_data_buffer = dev_data::DataBufferType<64>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface =
        MockHardwareIface{hardware_iface::EEPromChipType::ST_M24128_BF};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};
    GIVEN("data accessor correctly initializes") {
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);
        queue_client.messages.clear();
        THEN("accessor attemps to create data table entry") {
            subject.create_data_part(0x00, 0x04);
            // for the first one we just expect a write to the first segment of
            // the table and we expect a read to the table_tail value so it can
            // be incremented
            REQUIRE(queue_client.messages.size() == 2);
            auto write_message =
                std::get<message::WriteEepromMessage>(queue_client.messages[0]);
            REQUIRE(write_message.memory_address ==
                    addresses::data_address_begin);
            REQUIRE(write_message.length ==
                    static_cast<uint16_t>(
                        hardware_iface::EEPromAddressType::EEPROM_ADDR_16_BIT) *
                        2);
            auto read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[1]);
            REQUIRE(read_message.memory_address ==
                    addresses::lookup_table_tail_begin);
            REQUIRE(read_message.length == addresses::lookup_table_tail_length);
        }
    }
}

SCENARIO("writing to data partition") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto dev_data_buffer = dev_data::DataBufferType<64>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface = MockHardwareIface{};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    auto data_entry_key = 0x00;
    auto data_entry_length = 0x04;
    GIVEN("data accessor correctly initializes") {
        // initalize the accessor driver by reading the data table length and
        // config
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);

        // after the read_message callback is called the driver should write the
        // new tail to memory
        auto write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        auto data_tail_mock = types::EepromData{};
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        // add a data entry partition
        subject.create_data_part(data_entry_key, data_entry_length);
        // using eeprom data format for convience
        auto data_table_mock = types::EepromData{};
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[0]);
        // keep a copy of what the driver wrote to the lookup table so we can
        // send it to the read later.
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_table_mock.begin());
        // callback the read message so the tail can update
        read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[1]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data_tail_mock},
            read_message.callback_param);
        // update our data_tail_mock
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        THEN("Writing to the data partition") {
            auto data_value = std::array<uint8_t, 4>{0x01, 0x02, 0x03, 0x04};
            subject.write_data(data_entry_key, data_value);
            REQUIRE(queue_client.messages.size() == 1);
            // read back the data_table_entry
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            read_message.callback(
                message::EepromMessage{
                    .memory_address = read_message.memory_address,
                    .length = read_message.length,
                    .data = data_table_mock},
                read_message.callback_param);
            // after reading the table entry the driver should write the value
            // to the correct position
            REQUIRE(queue_client.messages.size() == 2);
            write_message =
                std::get<message::WriteEepromMessage>(queue_client.messages[1]);
            REQUIRE(write_message.memory_address ==
                    static_cast<uint16_t>(
                        hardware_iface::EEpromMemorySize::MICROCHIP_256_BYTE) -
                        data_entry_length);
            REQUIRE(write_message.length == data_entry_length);
            REQUIRE(
                std::equal(write_message.data.begin(),
                           write_message.data.begin() + write_message.length,
                           data_value.begin()));
        }
    }
}

SCENARIO("writing large data to partition") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    constexpr auto json_string = std::string_view(
        "{\"ulPerMm\":[{\"aspirate\":[[1.8263,-0.0958,1.088],[2.5222,-0.104,1."
        "1031],[3.2354,-0.0447,0.9536],[3.9984,-0.012,0.8477],[12.5135,-0.0021,"
        "0.8079]],\"dispense\":[[12.5135,0,0.7945]]},{\"aspirate\":[[1."
        "438649211,0.01931415115,0.691538317],[1.836824579,0.03868955123,0."
        "6636639129],[2.960052684,0.00470371018,0.7260899411],[4.487508789,0."
        "005175245625,0.7246941713],[10.59661421,0.001470408978,0.7413196584]],"
        "\"dispense\":[[12.5135,0,0.7945]]}]}");
    auto json_blob = std::array<unsigned char, json_string.size() + 1>{};
    std::copy_n(reinterpret_cast<const unsigned char*>(json_string.begin()),
                json_string.size(), json_blob.begin());
    auto dev_data_buffer = dev_data::DataBufferType<json_blob.size()>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface =
        MockHardwareIface{hardware_iface::EEPromChipType::ST_M24128_BF};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    uint16_t data_entry_key = 0x00;
    uint16_t data_entry_length = json_blob.size();
    GIVEN("data accessor correctly initializes") {
        // initalize the accessor driver by reading the data table length and
        // config
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);

        // after the read_message callback is called the driver should write the
        // new tail to memory
        auto write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        auto data_tail_mock = types::EepromData{};
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        // add a data entry partition
        subject.create_data_part(data_entry_key, data_entry_length);
        // using eeprom data format for convience
        auto data_table_mock = types::EepromData{};
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[0]);
        // keep a copy of what the driver wrote to the lookup table so we can
        // send it to the read later.
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_table_mock.begin());
        // callback the read message so the tail can update
        read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[1]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data_tail_mock},
            read_message.callback_param);
        // update our data_tail_mock
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        THEN("Writing to the data partition") {
            subject.write_data(data_entry_key, json_blob);
            REQUIRE(queue_client.messages.size() == 1);
            // read back the data_table_entry
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            read_message.callback(
                message::EepromMessage{
                    .memory_address = read_message.memory_address,
                    .length = read_message.length,
                    .data = data_table_mock},
                read_message.callback_param);
            // after reading the table entry the driver should queue a bunch of
            // writes
            REQUIRE(queue_client.messages.size() ==
                    1 + (json_blob.size() / types::max_data_length) +
                        (json_blob.size() % types::max_data_length != 0));
        }
    }
}

SCENARIO("reading from data partition") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    auto dev_data_buffer = dev_data::DataBufferType<64>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface = MockHardwareIface{};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    auto data_entry_key = 0x00;
    auto data_entry_length = 0x04;
    GIVEN("data accessor correctly initializes") {
        // initalize the accessor driver by reading the data table length and
        // config
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);

        // after the read_message callback is called the driver should write the
        // new tail to memory
        auto write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        auto data_tail_mock = types::EepromData{};
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        // add a data entry partition
        subject.create_data_part(data_entry_key, data_entry_length);
        // using eeprom data format for convience
        auto data_table_mock = types::EepromData{};
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[0]);
        // keep a copy of what the driver wrote to the lookup table so we can
        // send it to the read later.
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_table_mock.begin());
        // callback the read message so the tail can update
        read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[1]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data_tail_mock},
            read_message.callback_param);
        // update our data_tail_mock
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        THEN("reading from the data partition") {
            subject.get_data(data_entry_key, 1234);
            REQUIRE(queue_client.messages.size() == 1);
            // read back the data_table_entry
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            read_message.callback(
                message::EepromMessage{
                    .message_index = read_message.message_index,
                    .memory_address = read_message.memory_address,
                    .length = read_message.length,
                    .data = data_table_mock},
                read_message.callback_param);
            // after reading the table entry the driver should read the value
            // from the correct position
            REQUIRE(queue_client.messages.size() == 2);
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[1]);
            REQUIRE(read_message.memory_address ==
                    static_cast<uint16_t>(
                        hardware_iface::EEpromMemorySize::MICROCHIP_256_BYTE) -
                        data_entry_length);
            REQUIRE(read_message.length == data_entry_length);
            REQUIRE(read_message.message_index == 1234);
        }
    }
}

SCENARIO("reading large data from partition") {
    auto queue_client = MockEEPromTaskClient{};
    auto read_listener = MockListener{};
    constexpr auto json_string = std::string_view(
        "{\"ulPerMm\":[{\"aspirate\":[[1.8263,-0.0958,1.088],[2.5222,-0.104,1."
        "1031],[3.2354,-0.0447,0.9536],[3.9984,-0.012,0.8477],[12.5135,-0.0021,"
        "0.8079]],\"dispense\":[[12.5135,0,0.7945]]},{\"aspirate\":[[1."
        "438649211,0.01931415115,0.691538317],[1.836824579,0.03868955123,0."
        "6636639129],[2.960052684,0.00470371018,0.7260899411],[4.487508789,0."
        "005175245625,0.7246941713],[10.59661421,0.001470408978,0.7413196584]],"
        "\"dispense\":[[12.5135,0,0.7945]]}]}");
    auto json_blob = std::array<unsigned char, json_string.size() + 1>{};
    std::copy_n(reinterpret_cast<const unsigned char*>(json_string.begin()),
                json_string.size(), json_blob.begin());
    auto dev_data_buffer = dev_data::DataBufferType<json_blob.size()>{};
    auto subject =
        dev_data::DevDataAccessor{queue_client, read_listener, dev_data_buffer};

    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);
    auto hardware_iface =
        MockHardwareIface{hardware_iface::EEPromChipType::ST_M24128_BF};

    auto eeprom =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    auto data_entry_key = 0x00;
    auto data_entry_length = json_blob.size();
    GIVEN("data accessor correctly initializes") {
        // initalize the accessor driver by reading the data table length and
        // config
        auto data = types::EepromData{};
        data.fill(0x00);
        auto read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[0]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data},
            read_message.callback_param);
        auto config_msg = task::TaskMessage(
            std::get<message::ConfigRequestMessage>(queue_client.messages[1]));
        eeprom.handle_message(config_msg);

        // after the read_message callback is called the driver should write the
        // new tail to memory
        auto write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        auto data_tail_mock = types::EepromData{};
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        // add a data entry partition
        subject.create_data_part(data_entry_key, data_entry_length);
        // using eeprom data format for convience
        auto data_table_mock = types::EepromData{};
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[0]);
        // keep a copy of what the driver wrote to the lookup table so we can
        // send it to the read later.
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_table_mock.begin());
        // callback the read message so the tail can update
        read_message =
            std::get<message::ReadEepromMessage>(queue_client.messages[1]);
        read_message.callback(
            message::EepromMessage{
                .memory_address = read_message.memory_address,
                .length = read_message.length,
                .data = data_tail_mock},
            read_message.callback_param);
        // update our data_tail_mock
        write_message =
            std::get<message::WriteEepromMessage>(queue_client.messages[2]);
        std::copy_n(write_message.data.begin(), write_message.length,
                    data_tail_mock.begin());
        queue_client.messages.clear();
        THEN("reading from the data partition") {
            subject.get_data(data_entry_key, 1234);
            REQUIRE(queue_client.messages.size() == 1);
            // read back the data_table_entry
            read_message =
                std::get<message::ReadEepromMessage>(queue_client.messages[0]);
            read_message.callback(
                message::EepromMessage{
                    .message_index = read_message.message_index,
                    .memory_address = read_message.memory_address,
                    .length = read_message.length,
                    .data = data_table_mock},
                read_message.callback_param);
            // after reading the table entry the driver should queue a bunch of
            // reads
            REQUIRE(queue_client.messages.size() ==
                    1 + (json_blob.size() / types::max_data_length) +
                        (json_blob.size() % types::max_data_length != 0));
        }
    }
}
