#include <map>

#include "catch2/catch.hpp"
#include "common/simulation/i2c_sim.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/tasks/i2c_task.hpp"
#include "sensors/simulation/sensors.hpp"

class FakeSensor : public sensor_simulator::SensorType {
  public:
    FakeSensor() {
        ADDRESS = 0x1;
        DEVICE_ID = 0x2;
        REGISTER_MAP = {{0x2, 0}};
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
    std::array<uint8_t, 2> two_byte_arr{0x2, 0x0};
    uint16_t two_byte_data = 0x2;

    GIVEN("write command") {
        // make a copy of the two byte array before it's manipulated by
        // the i2c writer.
        auto empty_arr = two_byte_arr;
        writer.write(two_byte_data, ADDRESS);
        i2c_queue.try_read(&empty_msg);
        auto write_msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(write_msg);
        i2c.handle_message(converted_msg);

        sim_i2c.central_receive(empty_arr.data(), empty_arr.size(), ADDRESS, 1);
        REQUIRE(empty_arr[2] == 2);
    }
    GIVEN("read command") {
        uint8_t update = 0x0;
        auto callback = [&update](std::array<uint8_t, 5> value) -> void {
            update = value[2];
        };
        std::array<uint8_t, 2> data_to_store = {0x2, 0x3};
        sim_i2c.central_transmit(data_to_store.data(), 2, ADDRESS, 1);

        writer.read(ADDRESS, callback, 0x2);
        i2c_queue.try_read(&empty_msg);
        auto read_msg = std::get<i2c_writer::ReadFromI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(read_msg);
        i2c.handle_message(converted_msg);
        REQUIRE(update == data_to_store[1]);
    }
}