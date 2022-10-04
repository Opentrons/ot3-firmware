#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/hdc3020.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

template <typename Message, typename Queue>
requires std::constructible_from<i2c::poller::TaskMessage, Message>
auto get_message(Queue& q) -> Message {
    i2c::poller::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

constexpr auto sensor_id = can::ids::SensorId::S0;
constexpr uint8_t environment_id =
    static_cast<uint8_t>(can::ids::SensorType::environment);
constexpr uint8_t sensor_id_int = 0x0;

namespace sensors {

namespace tasks {

SCENARIO("Environment Sensor Task Functionality") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> i2c_poll_queue{};
    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>
        environment_queue{};
    test_mocks::MockI2CResponseQueue response_queue;

    i2c::poller::TaskMessage empty_poll_msg{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    auto queue_client = mock_client::QueueClient{.environment_sensor_queue =
                                                     &environment_queue};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&i2c_poll_queue);
    auto sensor = sensors::tasks::EnvironmentSensorMessageHandler{
        writer, poller, queue_client, response_queue, sensor_id};

    GIVEN("CAN messages accepted by the environment sensor task") {
        WHEN("the handler function receives a ReadFromSensorRequest") {
            auto read_environment = sensors::utils::TaskMessage(
                can::messages::ReadFromSensorRequest({}, environment_id));
            sensor.handle_message(read_environment);
            THEN(
                "the i2c queue is populated with a SingleRegisterPollRead "
                "command") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        i2c_poll_queue);

                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.first.address == hdc3020::ADDRESS);
                    REQUIRE(transact_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                hdc3020::Registers::TRIGGER_ON_DEMAND_MODE));
                    REQUIRE(transact_message.polling == 1);
                }
            }
        }
        WHEN("the handler function receives a BaselineSensorRequest") {
            auto read_baseline_environment = sensors::utils::TaskMessage(
                can::messages::BaselineSensorRequest({},0xdeadbeef, environment_id, 0, 5));
            sensor.handle_message(read_baseline_environment);
            THEN(
                "the i2c queue is populated with a SingleRegisterPollRead "
                "command") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        i2c_poll_queue);

                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.first.address == hdc3020::ADDRESS);
                    REQUIRE(transact_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                hdc3020::Registers::TRIGGER_ON_DEMAND_MODE));
                    REQUIRE(transact_message.polling == 5);
                }
            }
        }
        WHEN("the handler function receives a BindSensorOutputRequest") {
            auto bind_environment = sensors::utils::TaskMessage(
                can::messages::BindSensorOutputRequest(
                    {}, 0xdeadbeef, can::ids::SensorType::environment, sensor_id, 2));
            sensor.handle_message(bind_environment);
            THEN(
                "the i2c queue is populated with a "
                "ConfigureSingleRegisterContinuousPolling command") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
            }
            AND_WHEN("we read the message from the queue") {
                auto transact_message = get_message<
                    i2c::messages::ConfigureSingleRegisterContinuousPolling>(
                    i2c_poll_queue);

                THEN("The command and register addresses are correct") {
                    REQUIRE(transact_message.first.address == hdc3020::ADDRESS);
                    REQUIRE(transact_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                hdc3020::Registers::AUTO_MEASURE_10M1S));
                    // measure mode 1 for this register
                    REQUIRE(transact_message.first.write_buffer[1] == 0x21);
                    REQUIRE(transact_message.delay_ms == 100);
                }
            }
        }
    }
    GIVEN("CAN messages not accepted by the environment sensor task") {
        WHEN("the handler function receives a WriteToSensorRequest") {
            auto write_environment = sensors::utils::TaskMessage(
                can::messages::WriteToSensorRequest({},0xdeadbeef, environment_id, 0, 1));
            sensor.handle_message(write_environment);
            THEN("the i2c queue is not populated") {
                REQUIRE(i2c_poll_queue.get_size() == 0);
            }
        }
        WHEN("the handler function receives a SetSensorThresholdRequest") {
            auto threshold_environment = sensors::utils::TaskMessage(
                can::messages::SetSensorThresholdRequest(
                    {}, 0xdeadbeef, can::ids::SensorType::environment, sensor_id, 1,
                    can::ids::SensorThresholdMode::absolute));
            sensor.handle_message(threshold_environment);
            THEN("the i2c queue is not populated") {
                REQUIRE(i2c_poll_queue.get_size() == 0);
            }
        }
    }
}
}  // namespace tasks
}  // namespace sensors
