#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "motor-control/core/utils.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/utils.hpp"

SCENARIO("read capacitance sensor values") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensor_task_utils::TaskMessage>
        capacitive_queue{};

    i2c_writer::TaskMessage empty_msg{};
    auto queue_client =
        mock_client::QueueClient{.capacitive_sensor_queue = &capacitive_queue};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);

    auto sensor =
        capacitive_sensor_task::CapacitiveMessageHandler{writer, queue_client};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a request to take a single read of the capacitive sensor") {
        auto single_read = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, capacitive_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto read_message =
                    std::get<i2c_writer::MultiRegisterPollReadFromI2C>(
                        empty_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.address == fdc1004_utils::ADDRESS);
                    REQUIRE(read_message.register_buffer_1[0] ==
                            fdc1004_utils::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.register_buffer_2[0] ==
                            fdc1004_utils::LSB_MEASUREMENT_1);
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the capacitance sensor") {
        int NUM_READS = 30;
        auto multi_read = sensor_task_utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, capacitive_id, NUM_READS));
        sensor.handle_message(multi_read);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                i2c_queue.try_read(&empty_msg);
                auto read_message =
                    std::get<i2c_writer::MultiRegisterPollReadFromI2C>(
                        empty_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.address == fdc1004_utils::ADDRESS);
                    REQUIRE(read_message.register_buffer_1[0] ==
                            fdc1004_utils::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.register_buffer_2[0] ==
                            fdc1004_utils::LSB_MEASUREMENT_1);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    std::array<uint8_t, 5> buffer_a = {2, 8, 0, 0, 0};
                    std::array<uint8_t, 5> buffer_b = {1, 1, 0, 0, 0};
                    read_message.handle_buffer(buffer_a, buffer_b);
                    read_message.client_callback();
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 15);
                    float expected = 0.53542f;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }

    GIVEN("a drifting capacitance reading") {
        int NUM_READS = 30;
        auto multi_read = sensor_task_utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, capacitive_id, NUM_READS));
        WHEN("we call the capacitance handler") {
            sensor.handle_message(multi_read);
            std::array<uint8_t, 5> buffer_a = {200, 80, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {100, 10, 0, 0, 0};
            i2c_queue.try_read(&empty_msg);
            auto read_message =
                std::get<i2c_writer::MultiRegisterPollReadFromI2C>(empty_msg);
            for (int i = 0; i < NUM_READS; i++) {
                read_message.handle_buffer(buffer_a, buffer_b);
            }
            read_message.client_callback();
            THEN("it should adjust the offset accordingly") {
                // check for the offset
                auto read = sensor_task_utils::TaskMessage(
                    can_messages::ReadFromSensorRequest({}, capacitive_id, 1));
                sensor.handle_message(read);
                mock_message_writer::TaskMessage can_msg{};

                can_queue.try_read(&can_msg);
                auto response_msg =
                    std::get<can_messages::ReadFromSensorResponse>(
                        can_msg.message);
                float expected = 0.3418;
                float check_data =
                    fixed_point_to_float(response_msg.sensor_data, 15);
                REQUIRE(check_data >= Approx(expected).epsilon(1e-4));
            }
        }
    }
}