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
#include "sensors/simulation/mock_hardware.hpp"

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

constexpr auto sensor_id = can::ids::SensorId::S0;
constexpr uint8_t sensor_id_int = 0x0;

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
        writer, poller, queue_client, response_queue, mock_hw, sensor_id};
    constexpr uint8_t pressure_id = 0x4;
    constexpr uint8_t pressure_temperature_id = 0x5;

    GIVEN("a request to take a single read of the pressure sensor") {
        auto single_read =
            sensors::utils::TaskMessage(can::messages::ReadFromSensorRequest(
                {}, pressure_id, sensor_id_int));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c queue is populated with a MEASURE MODE 4 command") {
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("the data ready interrupt is triggered") {
                auto transact_message =
                    get_message<i2c::messages::Transact>(i2c_queue);
                THEN(
                    "the i2c queue is populated with a MEASURE MODE 4 "
                    "command") {
                    REQUIRE(transact_message.transaction.address ==
                            sensors::mmr920C04::ADDRESS);
                }
            }
        }
    }
    GIVEN("a request to take a single read of the temperature sensor") {
        auto single_read =
            sensors::utils::TaskMessage(can::messages::ReadFromSensorRequest(
                {}, pressure_temperature_id, sensor_id_int));
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
}
// TODO: implement and test polling for the pressure sensor