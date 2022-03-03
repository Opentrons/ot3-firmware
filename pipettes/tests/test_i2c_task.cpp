#include <map>

#include "catch2/catch.hpp"
#include "common/simulation/i2c_sim.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/tasks/i2c_task.hpp"
#include "sensors/simulation/sensors.hpp"
#include "sensors/tests/callbacks.hpp"

class FakeSensor : public sensor_simulator::SensorType {
  public:
    FakeSensor() {
        ADDRESS = 0x1;
        DEVICE_ID = 0x2;
        REGISTER_MAP = {{0x2, 0x3}, {0x5, 0x4}};
    }
};

auto fakesensor = FakeSensor{};

SCENARIO("read and write data to the i2c task") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    i2c_writer::TaskMessage empty_msg{};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    constexpr uint16_t ADDRESS = 0x1;

    std::map<uint16_t, sensor_simulator::SensorType> sensor_map = {
        {fakesensor.ADDRESS, fakesensor}};
    auto sim_i2c = sim_i2c::SimI2C{sensor_map};

    auto i2c = i2c_task::I2CMessageHandler{sim_i2c};
    auto single_update = mock_callbacks::UpdateCallback{};
    auto multi_update = mock_callbacks::MultiUpdateCallback{};

    GIVEN("write command") {
        std::array<uint8_t, 5> five_byte_arr{0x2, 0x0, 0x0, 0x0, 0x0};
        uint32_t write_data = 0x2010200;

        // make a copy of the two byte array before it's manipulated by
        // the i2c writer.
        writer.write(write_data, ADDRESS);
        i2c_queue.try_read(&empty_msg);
        auto write_msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(write_msg);
        i2c.handle_message(converted_msg);

        sim_i2c.central_receive(five_byte_arr.data(), five_byte_arr.size(),
                                ADDRESS, 1);
        REQUIRE(five_byte_arr[3] == 2);
    }
    GIVEN("read command") {
        single_update.reset();
        writer.read(
            ADDRESS, [&single_update]() { single_update.send_to_can(); },
            [&single_update](auto message_a) {
                single_update.handle_data(message_a);
            },
            0x2);
        i2c_queue.try_read(&empty_msg);
        auto read_msg = std::get<i2c_writer::ReadFromI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(read_msg);
        i2c.handle_message(converted_msg);
        REQUIRE(single_update.update_value == fakesensor.REGISTER_MAP[2]);
    }
    GIVEN("poll one register command") {
        constexpr int NUM_READS = 10;
        constexpr int DELAY_MS = 1;
        single_update.reset();

        writer.single_register_poll(
            ADDRESS, NUM_READS, DELAY_MS,
            [&single_update]() { single_update.send_to_can(); },
            [&single_update](auto message_a) {
                single_update.handle_data(message_a);
            },
            0x2);
        i2c_queue.try_read(&empty_msg);
        auto read_msg =
            std::get<i2c_writer::SingleRegisterPollReadFromI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(read_msg);
        i2c.handle_message(converted_msg);
        uint8_t expected_accumulated_data =
            fakesensor.REGISTER_MAP[2] * NUM_READS;
        REQUIRE(single_update.update_value == expected_accumulated_data);
    }

    GIVEN("poll two registers command") {
        constexpr int NUM_READS = 10;
        constexpr int DELAY_MS = 1;

        writer.multi_register_poll(
            ADDRESS, NUM_READS, DELAY_MS,
            [&multi_update]() { multi_update.send_to_can(); },
            [&multi_update](auto message_a, auto message_b) {
                multi_update.handle_data(message_a, message_b);
            },
            0x2, 0x5);
        i2c_queue.try_read(&empty_msg);
        auto read_msg =
            std::get<i2c_writer::MultiRegisterPollReadFromI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(read_msg);
        i2c.handle_message(converted_msg);
        uint8_t expected_accumulated_data_reg_a =
            fakesensor.REGISTER_MAP[2] * NUM_READS;
        uint8_t expected_accumulated_data_reg_b =
            fakesensor.REGISTER_MAP[5] * NUM_READS;
        REQUIRE(multi_update.register_a_value ==
                expected_accumulated_data_reg_a);
        REQUIRE(multi_update.register_b_value ==
                expected_accumulated_data_reg_b);
    }
}