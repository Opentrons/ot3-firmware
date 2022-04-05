#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "motor-control/core/utils.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "sensors/core/utils.hpp"

SCENARIO("read pressure sensor values") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c_poller::TaskMessage> i2c_poll_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensor_task_utils::TaskMessage>
        pressure_queue{};

    i2c_writer::TaskMessage empty_msg{};
    i2c_poller::TaskMessage empty_poll_msg{};
    auto queue_client =
        mock_client::QueueClient{.pressure_sensor_queue = &pressure_queue};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    auto poller = i2c_poller::I2CPoller<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&i2c_poll_queue);

    auto sensor = pressure_sensor_task::PressureMessageHandler{writer, poller,
                                                               queue_client};
    constexpr uint8_t pressure_id = 0x4;
    constexpr uint8_t pressure_temperature_id = 0x5;

    GIVEN("a request to take a single read of the pressure sensor") {
        auto single_read = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, pressure_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
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
                    REQUIRE(write_message.address == mmr920C04::ADDRESS);
                    REQUIRE(write_message.buffer[0] ==
                            static_cast<uint8_t>(
                                mmr920C04::Registers::PRESSURE_READ));
                    REQUIRE(read_message.address == mmr920C04::ADDRESS);
                }
            }
        }
    }
    GIVEN("a request to take a single read of the temperature sensor") {
        auto single_read = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, pressure_temperature_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
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
                    REQUIRE(write_message.address == mmr920C04::ADDRESS);
                    REQUIRE(write_message.buffer[0] ==
                            static_cast<uint8_t>(
                                mmr920C04::Registers::TEMPERATURE_READ));
                    REQUIRE(read_message.address == mmr920C04::ADDRESS);
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the pressure sensor") {
        int NUM_READS = 30;
        auto multi_read = sensor_task_utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, pressure_id, NUM_READS));
        sensor.handle_message(multi_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                i2c_poll_queue.try_read(&empty_poll_msg);
                auto read_message =
                    std::get<i2c_poller::SingleRegisterPollReadFromI2C>(
                        empty_poll_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.address == mmr920C04::ADDRESS);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> buffer_a = {0, 8, 2, 8, 0};
                    read_message.handle_buffer(buffer_a);
                    read_message.client_callback();
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 15);
                    // pressure value returned in pascals
                    float expected = 257.33041;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
}
