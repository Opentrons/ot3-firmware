#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "motor-control/core/utils.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/core/hdc2080.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

SCENARIO("read temperature and humidity values") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensor_task_utils::TaskMessage>
        environment_queue{};

    i2c_writer::TaskMessage empty_msg{};
    auto queue_client = mock_client::QueueClient{.environment_sensor_queue =
                                                     &environment_queue};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);

    auto sensor = environment_sensor_task::EnvironmentSensorMessageHandler{
        writer, queue_client};
    constexpr uint8_t humidity_id = 0x2;
    constexpr uint8_t temperature_id = 0x3;

    GIVEN("a request to read the humidity of the sensor") {
        auto read_humidity = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, humidity_id));
        sensor.handle_message(read_humidity);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto transact_message =
                    std::get<i2c_writer::TransactWithI2C>(empty_msg);

                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(transact_message.buffer[0] ==
                            hdc2080_utils::LSB_HUMIDITY_REGISTER);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> my_buff = {250, 80, 0, 0, 0};
                    transact_message.handle_buffer(my_buff);
                    transact_message.client_callback();
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 15);
                    float expected = 48.88916;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
    GIVEN("a request to read the temperature of the sensor") {
        auto read_temperature = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, temperature_id));
        sensor.handle_message(read_temperature);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto transact_message =
                    std::get<i2c_writer::TransactWithI2C>(empty_msg);
                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.address == hdc2080_utils::ADDRESS);
                    REQUIRE(transact_message.buffer[0] ==
                            hdc2080_utils::LSB_TEMPERATURE_REGISTER);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> my_buff = {200, 0, 0, 0, 0};
                    transact_message.handle_buffer(my_buff);
                    transact_message.client_callback();
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 15);
                    float expected = 44.20312;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
}
