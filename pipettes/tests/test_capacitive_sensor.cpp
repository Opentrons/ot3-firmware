#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "motor-control/core/utils.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/messages.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/utils.hpp"
#include "sensors/tests/mock_hardware.hpp"

SCENARIO("read capacitance sensor values") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c_poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensor_task_utils::TaskMessage>
        capacitive_queue{};

    i2c_writer::TaskMessage empty_msg{};
    i2c_poller::TaskMessage empty_poll_msg{};
    auto queue_client =
        mock_client::QueueClient{.capacitive_sensor_queue = &capacitive_queue};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    auto poller = i2c_poller::I2CPoller<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&poller_queue);

    auto sensor = capacitive_sensor_task::CapacitiveMessageHandler{
        writer, poller, mock_hw, queue_client};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a request to take a single read of the capacitive sensor") {
        auto single_read = sensor_task_utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, capacitive_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c poller queue is populated with a poll request") {
                REQUIRE(poller_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                poller_queue.try_read(&empty_poll_msg);
                auto read_message =
                    std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                        empty_poll_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.address == fdc1004_utils::ADDRESS);
                    REQUIRE(read_message.register_1_buffer[0] ==
                            fdc1004_utils::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.register_2_buffer[0] ==
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
            THEN("the poller queue is populated with a poll request") {
                REQUIRE(poller_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                poller_queue.try_read(&empty_poll_msg);
                auto read_message =
                    std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                        empty_poll_msg);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.address == fdc1004_utils::ADDRESS);
                    REQUIRE(read_message.register_1_buffer[0] ==
                            fdc1004_utils::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.register_2_buffer[0] ==
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
                    float expected = 1.08333;
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
            poller_queue.try_read(&empty_poll_msg);
            auto read_message =
                std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                    empty_poll_msg);
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

SCENARIO("capacitance callback tests") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c_poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};

    auto queue_client = mock_client::QueueClient{};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    capacitance_callbacks::ReadCapacitanceCallback callback_host(
        queue_client, writer, mock_hw, 1, 0);

    mock_message_writer::TaskMessage empty_msg{};

    GIVEN("a callback instance that is echoing and not bound") {
        callback_host.set_echoing(true);
        callback_host.set_bind_sync(false);

        WHEN("it receives data under its threshold") {
            std::array<uint8_t, 5> buffer_a = {0, 0, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {0, 0, 0, 0, 0};

            callback_host.handle_data_ongoing(buffer_a, buffer_b);

            THEN("it should forward the converted data via can") {
                REQUIRE(can_queue.has_message());
            }
            THEN("it should not touch the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == false);
                REQUIRE(mock_hw.get_sync_set_calls() == 0);
                REQUIRE(mock_hw.get_sync_reset_calls() == 0);
            }
        }
        WHEN("it receives data over its threshold") {
            std::array<uint8_t, 5> buffer_a = {200, 80, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {100, 10, 0, 0, 0};

            callback_host.handle_data_ongoing(buffer_a, buffer_b);

            THEN("it should forward the converted data via can") {
                can_queue.try_read(&empty_msg);
                auto sent = std::get<can_messages::ReadFromSensorResponse>(
                    empty_msg.message);
                REQUIRE(sent.sensor == can_ids::SensorType::capacitive);
                // we're just checking that the data is faithfully represented,
                // don't really care what it is
                REQUIRE(sent.sensor_data == 210044480);
            }
            THEN("it should not touch the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == false);
                REQUIRE(mock_hw.get_sync_set_calls() == 0);
                REQUIRE(mock_hw.get_sync_reset_calls() == 0);
            }
        }
    }
    GIVEN("a callback instance that is bound and not echoing") {
        callback_host.set_echoing(false);
        callback_host.set_bind_sync(true);

        WHEN("it receives data under its threshold") {
            std::array<uint8_t, 5> buffer_a = {0, 0, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {0, 0, 0, 0, 0};

            callback_host.handle_data_ongoing(buffer_a, buffer_b);

            THEN("it should not send can messages") {
                REQUIRE(!can_queue.has_message());
            }
            THEN("it should deassert the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == false);
                REQUIRE(mock_hw.get_sync_set_calls() == 0);
                REQUIRE(mock_hw.get_sync_reset_calls() == 1);
            }
        }
        WHEN("it receives data over its threshold") {
            std::array<uint8_t, 5> buffer_a = {200, 80, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {100, 10, 0, 0, 0};

            callback_host.handle_data_ongoing(buffer_a, buffer_b);

            THEN("it should not send can messages") {
                REQUIRE(!can_queue.has_message());
            }
            THEN("it should assert the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == true);
                REQUIRE(mock_hw.get_sync_set_calls() == 1);
                REQUIRE(mock_hw.get_sync_reset_calls() == 0);
            }
        }
    }
}
