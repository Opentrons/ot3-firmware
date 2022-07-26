#include <concepts>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/tasks/pressure_driver.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "sensors/core/utils.hpp"
#include "sensors/simulation/mock_hardware.hpp"
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
SCENARIO("Read pressure sensor values") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> i2c_poll_queue{};
    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage> pressure_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    test_mocks::MockSensorHardware mock_hw{};

    i2c::writer::TaskMessage empty_msg{};
    i2c::poller::TaskMessage empty_poll_msg{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    test_mocks::MockSensorHardware hardware{};
    auto queue_client =
        mock_client::QueueClient{.pressure_sensor_queue = &pressure_queue};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&i2c_poll_queue);
    sensors::tasks::MMR920C04 driver(writer, poller, queue_client,
                                     pressure_queue, hardware, sensor_id);

    GIVEN("Pressure read with SensorMode = SINGLE_READ") {
        driver.set_sync_bind(can::ids::SensorOutputBinding::report);
        driver.set_limited_poll(true);
        WHEN("the sensor_callback function is called") {
            driver.sensor_callback();
            THEN(
                "the i2c queue is populated with a READ_PRESSURE command and a "
                "RESET command") {
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
                    "the CAN bus, and sync state is false") {
                    can::message_writer_task::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    float expected = 33.53677;
                    REQUIRE(check_data == Approx(expected));
                    REQUIRE(hardware.get_sync_state_mock() == false);
                }
            }
        }
    }
    GIVEN(
        "Pressure read with SensorMode = POLLING, and output binding = sync") {
        driver.set_limited_poll(false);
        driver.get_pressure();
        driver.set_sync_bind(can::ids::SensorOutputBinding::sync);
        WHEN("the sensor_callback function is called") {
            driver.sensor_callback();
            THEN(
                "the i2c queue is populated with a MEASURE_MODE_4 and "
                "a READ_PRESSURE command only") {
                REQUIRE(i2c_queue.get_size() == 2);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
                auto reset_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(reset_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::PRESSURE_READ));
            }
            AND_WHEN(
                "the driver receives a response higher than the threshold") {
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
                    "no data is sent via the CAN bus, and the sync pin is "
                    "set") {
                    REQUIRE(can_queue.get_size() == 0);

                    REQUIRE(hardware.get_sync_state_mock() == true);
                }
            }
        }
    }
    GIVEN("Pressure read with output binding = report") {
//        driver.set_number_of_reads(2);
//        driver.get_pressure();
//        driver.set_sync_bind(can::ids::SensorOutputBinding::report);

        // limited poll
        WHEN("a limited poll for 3 reads is set") {
            driver.set_number_of_reads(4);
            driver.set_limited_poll(true);
            driver.set_sync_bind(can::ids::SensorOutputBinding::report);
            driver.get_pressure();
            THEN("the i2c queue receives a MEASURE_MODE_4 command") {
                REQUIRE(i2c_queue.get_size() == 1);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
            }
            THEN ("for each read, the i2c queue receives a READ_PRESSURE command") {
                for(int i = 0; i < 4; i++) {
                    driver.sensor_callback();
                    REQUIRE(i2c_queue.get_size() == 1);
                    auto read_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(read_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::PRESSURE_READ));
                }
            }
        }

        // single read
        // continuous poll
        // another single read

        /*
        WHEN("the sensor_callback function is called") {
            driver.sensor_callback();
            THEN(
                "the i2c queue is populated with a MEASURE_MODE_4 and "
                "a READ_PRESSURE command only") {
                REQUIRE(i2c_queue.get_size() == 2);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
                auto reset_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(reset_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::PRESSURE_READ));
            }
            AND_WHEN(
                "the driver receives a response higher than the threshold") {
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
                    "the CAN bus, and the sync pin is not set") {
                    can::message_writer_task::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data =
                        fixed_point_to_float(response_msg.sensor_data, 16);
                    float expected = 33.53677;
                    REQUIRE(check_data == Approx(expected));
                    REQUIRE(hardware.get_sync_state_mock() == false);
                }
            }
        }
        */
    }
}