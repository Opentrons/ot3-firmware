#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/hdc2080.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

template <typename Message, typename Queue>
auto get_message(Queue& q) -> Message {
    i2c::writer::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

SCENARIO("read temperature and humidity values") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>
        environment_queue{};
    test_mocks::MockI2CResponseQueue response_queue;

    i2c::writer::TaskMessage empty_msg{};
    auto queue_client = mock_client::QueueClient{.environment_sensor_queue =
                                                     &environment_queue};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);

    auto sensor = sensors::tasks::EnvironmentSensorMessageHandler{
        writer, queue_client, response_queue};
    constexpr uint8_t humidity_id = 0x2;
    constexpr uint8_t temperature_id = 0x3;

    GIVEN("a request to read the humidity of the sensor") {
        auto read_humidity = sensors::utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, humidity_id));
        sensor.handle_message(read_humidity);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message =
                    get_message<i2c::messages::Transact>(i2c_queue);

                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.transaction.address ==
                            sensors::hdc2080::ADDRESS);
                    REQUIRE(transact_message.transaction.write_buffer[0] ==
                            sensors::hdc2080::LSB_HUMIDITY_REGISTER);
                    REQUIRE(transact_message.transaction.bytes_to_write == 1);
                    REQUIRE(transact_message.transaction.bytes_to_read == 2);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    auto my_buff =
                        i2c::messages::MaxMessageBuffer{250, 80, 0, 0, 0};
                    auto response = test_mocks::launder_response(
                        transact_message, response_queue,
                        test_mocks::dummy_response(transact_message, my_buff));
                    sensor.handle_message(response);

                    mock_message_writer::TaskMessage can_msg{};
                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    float expected = 97.77832;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
    GIVEN("a request to read the temperature of the sensor") {
        auto read_temperature = sensors::utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, temperature_id));
        sensor.handle_message(read_temperature);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message =
                    get_message<i2c::messages::Transact>(i2c_queue);
                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.transaction.address ==
                            sensors::hdc2080::ADDRESS);
                    REQUIRE(transact_message.transaction.write_buffer[0] ==
                            sensors::hdc2080::LSB_TEMPERATURE_REGISTER);
                    REQUIRE(transact_message.transaction.bytes_to_write == 1);
                    REQUIRE(transact_message.transaction.bytes_to_read == 2);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    auto my_buff =
                        i2c::messages::MaxMessageBuffer{200, 0, 0, 0, 0};
                    auto response = test_mocks::launder_response(
                        transact_message, response_queue,
                        test_mocks::dummy_response(transact_message, my_buff));
                    sensor.handle_message(response);
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    float expected = 88.40625;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
}
