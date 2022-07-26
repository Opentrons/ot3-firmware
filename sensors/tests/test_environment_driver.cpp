#include <concepts>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/tasks/environment_driver.hpp"
#include "sensors/core/utils.hpp"
#include "sensors/simulation/hardware.hpp"
#include "sensors/tests/mock_hardware.hpp"
/*
 * NOTE: pressure_sensor_task.hpp is included here just because
 * the linter throws an unused-function error and NOLINT doesn't
 * seem to fix it
 * */

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

// TODO: simulate and test data_ready interrupt and response from sensor
SCENARIO("Read a single value from environment sensor") {
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> i2c_poll_queue{};
    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage> environment_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};

    i2c::poller::TaskMessage empty_poll_msg{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    auto queue_client =
        mock_client::QueueClient{.environment_sensor_queue = &environment_queue};
    queue_client.set_queue(&can_queue);
    poller.set_queue(&i2c_poll_queue);
    sensors::tasks::HDC3020 driver(poller, queue_client,
                                   environment_queue, sensor_id);

    GIVEN("A call to trigger on demand") {
        WHEN("a single read occurs when given 1 trigger read") {
            driver.trigger_on_demand();
            THEN(
                "we receive two messages") {
                REQUIRE(i2c_queue.get_size() == 2);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::PRESSURE_READ));
                auto reset_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(
                    reset_command.transaction.write_buffer[0] ==
                    static_cast<uint8_t>(sensors::mmr920C04::Registers::RESET));
            }
            AND_WHEN("the driver receives a response") {
                auto id = i2c::messages::TransactionIdentifier{
                    .token = static_cast<uint32_t>(
                        sensors::mmr920C04::Registers::PRESSURE_READ),
                    .is_completed_poll = false,
                    .transaction_index = static_cast<uint8_t>(0)};
                auto sensor_response = i2c::messages::TransactionResponse{
                    .id = id,
                    .bytes_read = 3,
                    .read_buffer = {0x0, 0x85, 0x96, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0}};
                driver.handle_response(sensor_response);
                THEN(
                    "the handle_message function sends the correct data via "
                    "the CAN bus") {
                    can::message_writer_task::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    float expected = 33.53677;
                    REQUIRE(check_data == Approx(expected));
                }
            }
        }
    }
}