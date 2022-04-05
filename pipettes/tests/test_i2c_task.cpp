#include <iostream>
#include <map>

#include "catch2/catch.hpp"
#include "common/simulation/i2c_sim.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/messages.hpp"
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

SCENARIO("read and write data to the i2c task") {
    auto fakesensor = FakeSensor{};
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

    GIVEN("write command") {
        std::array<uint8_t, 5> five_byte_arr{0x2, 0x0, 0x0, 0x0, 0x0};
        uint16_t write_data = 0x0122;

        // make a copy of the two byte array before it's manipulated by
        // the i2c writer.
        writer.write(ADDRESS, 0x2, write_data);
        i2c_queue.try_read(&empty_msg);
        auto write_msg = std::get<pipette_messages::WriteToI2C>(empty_msg);
        auto converted_msg = i2c_writer::TaskMessage(write_msg);
        i2c.handle_message(converted_msg);

        sim_i2c.central_receive(five_byte_arr.data(), five_byte_arr.size(),
                                ADDRESS, 1);
        REQUIRE(five_byte_arr[0] == 0);
        REQUIRE(five_byte_arr[1] == 0);
        REQUIRE(five_byte_arr[2] == 0x01);
        REQUIRE(five_byte_arr[3] == 0x22);
        REQUIRE(five_byte_arr[4] == 0);
    }
    GIVEN("read command") {
        single_update.reset();
        writer.write(ADDRESS, 0x02000000);
        writer.read(
            ADDRESS, [&single_update]() { single_update.send_to_can(); },
            [&single_update](auto message_a) {
                single_update.handle_data(message_a);
            });
        i2c_queue.try_read(&empty_msg);
        i2c.handle_message(empty_msg);
        REQUIRE(single_update.update_value == fakesensor.REGISTER_MAP.at(2));
    }
    GIVEN("transact command") {
        single_update.reset();
        writer.transact(
            ADDRESS, 0x02000000,
            [&single_update]() { single_update.send_to_can(); },
            [&single_update](auto message_a) {
                single_update.handle_data(message_a);
            });
        i2c_queue.try_read(&empty_msg);
        i2c.handle_message(empty_msg);
        REQUIRE(single_update.update_value == fakesensor.REGISTER_MAP[2]);
    }
}
