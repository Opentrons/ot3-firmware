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

    GIVEN("A single read of the pressure sensor, with output set to report") {
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
    GIVEN("An unlimited poll with sensor binding set to sync") {
        can_queue.reset();
        driver.set_limited_poll(false);
        driver.set_threshold(convert_to_fixed_point(15.0F, S15Q16_RADIX));
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
                    .read_buffer = {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                    0xFF, 0xFF}};
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
    GIVEN("output binding = report") {
        driver.set_sync_bind(can::ids::SensorOutputBinding::report);
        WHEN("a limited poll for 3 reads is set") {
            i2c_queue.reset();
            driver.set_number_of_reads(4);
            driver.set_limited_poll(true);
            driver.get_pressure();
            THEN("the i2c queue receives a MEASURE_MODE_4 command") {
                REQUIRE(i2c_queue.get_size() == 1);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
            }
            THEN(
                "for each read, the i2c queue receives a READ_PRESSURE "
                "command, RESET is sent after poll finishes") {
                for (int i = 0; i < 4; i++) {
                    i2c_queue.reset();
                    driver.sensor_callback();
                    auto read_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(read_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::PRESSURE_READ));
                }
                AND_THEN("a reset command is sent when the poll is finished") {
                    auto reset_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(reset_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::RESET));
                }
            }
        }
        WHEN("A single read command is sent") {
            driver.set_limited_poll(true);
            driver.set_number_of_reads(1);
            driver.get_pressure();
            THEN("the i2c queue receives a MEASURE_MODE_4 command") {
                REQUIRE(i2c_queue.get_size() == 1);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
            }
            AND_WHEN("the sensor callback ist triggered") {
                i2c_queue.reset();
                driver.sensor_callback();
                THEN(
                    "the i2c queue receives a PRESSURE_READ command, and a "
                    "RESET command") {
                    REQUIRE(i2c_queue.get_size() == 2);
                    auto read_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(read_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::PRESSURE_READ));
                    auto reset_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(reset_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::RESET));
                }
            }
        }
        WHEN("a continuous poll is requested") {
            driver.set_sync_bind(can::ids::SensorOutputBinding::sync);
            driver.set_limited_poll(false);
            driver.get_pressure();
            THEN(
                "the i2c queue receives a MEASURE_MODE_4 command, and then "
                "continuous PRESSURE_READ commands") {
                REQUIRE(i2c_queue.get_size() == 1);
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
                AND_THEN(
                    "continuous PRESSURE_READS commands are sent, with no "
                    "RESET") {
                    for (int i = 0; i < 10; i++) {
                        driver.sensor_callback();
                        REQUIRE(i2c_queue.get_size() == 1);
                        auto read_command =
                            get_message<i2c::messages::Transact>(i2c_queue);
                        REQUIRE(
                            read_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::PRESSURE_READ));
                    }
                }
            }
        }

        WHEN(
            "another single read is requested, and the sensor callback is "
            "executed") {
            driver.set_sync_bind(can::ids::SensorOutputBinding::report);
            driver.set_limited_poll(true);
            driver.set_number_of_reads(1);
            driver.get_pressure();
            driver.sensor_callback();
            THEN(
                "the i2c queue is populated with a MEASURE_MODE_4 and "
                "a READ_PRESSURE command") {
                REQUIRE(i2c_queue.get_size() == 3);
                auto measure_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(measure_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::MEASURE_MODE_4));
                auto read_command =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(read_command.transaction.write_buffer[0] ==
                        static_cast<uint8_t>(
                            sensors::mmr920C04::Registers::PRESSURE_READ));
                AND_THEN("A reset command is sent after the first read") {
                    auto reset_command =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(reset_command.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::mmr920C04::Registers::RESET));
                }
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
    }
    GIVEN("a baseline sensor request with 10 reads is processed") {
        int num_reads = 10;
        driver.set_sync_bind(can::ids::SensorOutputBinding::none);
        driver.set_limited_poll(true);
        driver.set_baseline_values(num_reads);
        driver.get_pressure();
        WHEN("pressure driver receives the requested sensor readings") {
            auto id = i2c::messages::TransactionIdentifier{
                .token = static_cast<uint32_t>(
                    sensors::mmr920C04::Registers::PRESSURE_READ),
                .is_completed_poll = false,
                .transaction_index = static_cast<uint8_t>(0)};
            for (int i = 0; i < 5; i++) {
                auto sensor_response = i2c::messages::TransactionResponse{
                    .id = id,
                    .bytes_read = 3,
                    .read_buffer = {0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0}};
                driver.handle_response(sensor_response);
            }
            for (int i = 0; i < 5; i++) {
                auto sensor_response = i2c::messages::TransactionResponse{
                    .id = id,
                    .bytes_read = 3,
                    .read_buffer = {0x20, 0x0, 0x0, 0x0 0x0, 0x0, 0x0,
                                    0x0, 0x0}};
                driver.handle_response(sensor_response);
            }
        }
        THEN("a BaselineSensorResponse is sent with the correct calculated average") {
            can::message_writer_task::TaskMessage can_msg{};

            can_queue.try_read(&can_msg);
            auto response_msg =
                std::get<can::messages::BaselineSensorResponse>(
                    can_msg.message);
            float check_data =
                fixed_point_to_float(response_msg.offset_average, 16);
            float expected = 30.0;
            REQUIRE(check_data == Approx(expected));
        }
    }
}
