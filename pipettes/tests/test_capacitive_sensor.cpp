#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/utils.hpp"

float fixed_to_float(uint32_t data) {
    constexpr uint32_t power_of_two = 2 << 15;
    return (1.0 * static_cast<float>(data)) / power_of_two;
}

SCENARIO("read temperature and humidity values") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensor_task_utils::TaskMessage>
        capacitive_queue{};

    i2c_writer::TaskMessage empty_msg{};
    auto queue_client = mock_client::QueueClient{.capacitive_sensor_queue =
    &capacitive_queue};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);

    auto sensor = environment_sensor_task::CapacitiveSensorMessageHandler{
        writer, queue_client};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a request to take a single read of the sensor") {
        auto single_read = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, capacitive_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_queue.get_size() == 2);
            }
            AND_WHEN("we read the messages from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto read_message =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                i2c_queue.try_read(&empty_msg);
                auto write_message =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(write_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(write_message.buffer[0] ==
                            hdc2080_utils::LSB_HUMIDITY_REGISTER);
                    REQUIRE(read_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(read_message.buffer[0] ==
                            hdc2080_utils::LSB_HUMIDITY_REGISTER);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> my_buff = {250, 80, 0, 0, 0};
                    read_message.client_callback(my_buff);
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = fixed_to_float(response_msg.sensor_data);
                    float expected = 48.88916;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the sensor") {
        int NUM_READS = 30;
        auto multi_read = sensor_task_utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, capacitive_id, NUM_READS));
        sensor.handle_message(multi_read);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_queue.get_size() == 2);
            }
            AND_WHEN("we read the messages from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto read_message =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                i2c_queue.try_read(&empty_msg);
                auto write_message =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(write_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(write_message.buffer[0] ==
                            hdc2080_utils::LSB_TEMPERATURE_REGISTER);
                    REQUIRE(read_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(read_message.buffer[0] ==
                            hdc2080_utils::LSB_TEMPERATURE_REGISTER);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> my_buff = {200, 0, 0, 0, 0};
                    read_message.client_callback(my_buff);
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = fixed_to_float(response_msg.sensor_data);
                    float expected = 44.20312;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
}