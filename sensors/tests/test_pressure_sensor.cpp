#include <concepts>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "sensors/core/utils.hpp"
#include "sensors/tests/mock_hardware.hpp"

template <typename Message, typename Queue>
requires std::constructible_from<i2c::poller::TaskMessage, Message>
auto get_message(Queue& q) -> Message {
    i2c::poller::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

template <typename Message, typename Queue>
requires std::constructible_from<i2c::writer::TaskMessage, Message>
auto get_message(Queue& q) -> Message {
    i2c::writer::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

SCENARIO("read pressure sensor values") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> i2c_poll_queue{};
    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage> pressure_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    test_mocks::MockSensorHardware mock_hw{};

    i2c::writer::TaskMessage empty_msg{};
    i2c::poller::TaskMessage empty_poll_msg{};
    auto queue_client =
        mock_client::QueueClient{.pressure_sensor_queue = &pressure_queue};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&i2c_poll_queue);

    auto sensor = sensors::tasks::PressureMessageHandler{
        writer, poller, queue_client, response_queue, mock_hw};
    constexpr uint8_t pressure_id = 0x4;
    constexpr uint8_t pressure_temperature_id = 0x5;

    GIVEN("a request to take a single read of the pressure sensor") {
        auto single_read = sensors::utils::TaskMessage(
            can::messages::ReadFromSensorRequest({}, pressure_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto transact_message =
                    get_message<i2c::messages::Transact>(i2c_queue);
                THEN("The command addresses are correct") {
                    REQUIRE(transact_message.transaction.address ==
                            sensors::mmr920C04::ADDRESS);
                    REQUIRE(transact_message.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::PRESSURE_READ));
                }
            }
        }
    }
    GIVEN("a request to take a single read of the temperature sensor") {
        auto single_read = sensors::utils::TaskMessage(
            can::messages::ReadFromSensorRequest({}, pressure_temperature_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a transact command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message =
                    get_message<i2c::messages::Transact>(i2c_queue);

                THEN("The command addresses are correct") {
                    REQUIRE(transact_message.transaction.address ==
                            sensors::mmr920C04::ADDRESS);
                    REQUIRE(
                        transact_message.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::TEMPERATURE_READ));
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the pressure sensor") {
        int NUM_READS = 30;
        auto multi_read = sensors::utils::TaskMessage(
            can::messages::BaselineSensorRequest({}, pressure_id, NUM_READS));
        sensor.handle_message(multi_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a write and read command") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        i2c_poll_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::mmr920C04::ADDRESS);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                THEN(
                    "using the callback with data returns the expected value") {
                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0, 8, 2, 8, 0};
                    auto response = test_mocks::launder_response(
                        read_message, response_queue,
                        test_mocks::dummy_single_response(read_message, true,
                                                          buffer_a));
                    sensor.handle_message(response);
                    can::message_writer_task::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    // pressure value returned in pascals
                    float expected = 514.66083;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }
}
