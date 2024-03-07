#include <concepts>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/mmr920.hpp"
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
static std::array<float, PRESSURE_SENSOR_BUFFER_SIZE> p_buff;

SCENARIO("Testing the pressure sensor driver") {
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
    sensors::tasks::MMR920 driver(
        writer, poller, queue_client, pressure_queue, hardware, sensor_id,
        sensors::mmr920::SensorVersion::mmr920c04, &p_buff);

    can::message_writer_task::TaskMessage empty_can_msg{};

    GIVEN("A single read request the pressure sensor") {
        const uint8_t tags_as_int = 0x1;
        i2c::messages::TransactionResponse message{
            .id =
                i2c::messages::TransactionIdentifier{
                    .token = sensors::utils::build_id(
                        sensors::mmr920::ADDRESS,
                        static_cast<uint8_t>(
                            sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                        0x1),
                    .is_completed_poll = 1,
                    .transaction_index = 0},
            .bytes_read = 3,
            .read_buffer = {0x3D, 0x09, 0x00}};  // 40 cmH20
        WHEN("the the limited poll is initiated") {
            driver.poll_limited_pressure(1, tags_as_int);
            THEN("the poller queue is populated with a PRESSURE_READ") {
                REQUIRE(i2c_queue.get_size() == 0);
                REQUIRE(i2c_poll_queue.get_size() == 1);
                auto poller_command =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        i2c_poll_queue);
                REQUIRE(
                    poller_command.first.write_buffer[0] ==
                    static_cast<uint8_t>(
                        sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ));
            }
        }
        WHEN("A response is sent to the handle baseline function") {
            driver.handle_baseline_pressure_response(message);
            THEN(
                "A ReadFromSensorResponse is sent to the CAN queue and it's "
                "the expected value") {
                can_queue.try_read(&empty_can_msg);
                REQUIRE(std::holds_alternative<
                        can::messages::ReadFromSensorResponse>(
                    empty_can_msg.message));

                auto response_msg =
                    std::get<can::messages::ReadFromSensorResponse>(
                        empty_can_msg.message);
                float check_data_pascals =
                    fixed_point_to_float(response_msg.sensor_data, 16);
                float expected_pascals = 3922.66;
                REQUIRE(check_data_pascals == Approx(expected_pascals));
                REQUIRE(hardware.get_sync_state_mock() == false);
            }
        }
        WHEN("The response data is a negative value") {
            message.read_buffer = {0xF8, 0x5E, 0xE0};  // -5 cmH20
            driver.handle_baseline_pressure_response(message);
            THEN("We get the expected negative pressure read") {
                can_queue.try_read(&empty_can_msg);
                auto response_msg =
                    std::get<can::messages::ReadFromSensorResponse>(
                        empty_can_msg.message);
                float check_data_pascals =
                    signed_fixed_point_to_float(response_msg.sensor_data, 16);
                float expected_pascals = -490.3325;
                REQUIRE(check_data_pascals == Approx(expected_pascals));
            }
        }
    }

    GIVEN("a completed limited poll message with a baseline tag") {
        driver.set_echoing(true);

        std::array tags_for_baseline{
            sensors::utils::ResponseTag::IS_PART_OF_POLL,
            sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
        i2c::messages::TransactionResponse message{
            .id =
                i2c::messages::TransactionIdentifier{
                    .token = sensors::utils::build_id(
                        sensors::mmr920::ADDRESS,
                        static_cast<uint8_t>(
                            sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                        sensors::utils::byte_from_tags(tags_for_baseline)),
                    .is_completed_poll = 1,
                    .transaction_index = 0},
            .bytes_read = 3,
            .read_buffer = {0x3D, 0x09, 0x00}};  // 40 cmH20
        WHEN("a completed poll is sent to handle baseline response") {
            driver.handle_baseline_pressure_response(message);
            THEN("the can queue is populated with a baseline sensor request") {
                can_queue.try_read(&empty_can_msg);
                REQUIRE(std::holds_alternative<
                        can::messages::BaselineSensorResponse>(
                    empty_can_msg.message));
            }
        }
    }

    GIVEN("An unlimited poll with sensor binding set to sync") {
        driver.set_threshold(15.0F, can::ids::SensorThresholdMode::absolute, 1,
                             false);

        driver.set_echoing(false);
        driver.set_bind_sync(true);
        std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                        sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
        auto tags_as_int = sensors::utils::byte_from_tags(tags);
        WHEN("the continuous poll function is called") {
            driver.poll_continuous_pressure(tags_as_int);
            THEN("a READ_PRESSURE command only") {
                REQUIRE(i2c_queue.get_size() == 0);
                REQUIRE(i2c_poll_queue.get_size() == 1);
                auto read_command = get_message<
                    i2c::messages::ConfigureSingleRegisterContinuousPolling>(
                    i2c_poll_queue);
                REQUIRE(
                    read_command.first.write_buffer[0] ==
                    static_cast<uint8_t>(
                        sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ));
            }
        }
        WHEN("the driver receives a response higher than the threshold") {
            auto id = i2c::messages::TransactionIdentifier{
                .token = sensors::utils::build_id(
                    sensors::mmr920::ADDRESS,
                    static_cast<uint8_t>(
                        sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                    tags_as_int),
                .is_completed_poll = false,
                .transaction_index = static_cast<uint8_t>(0)};
            auto sensor_response = i2c::messages::TransactionResponse{
                .id = id, .bytes_read = 3, .read_buffer = {0x7F, 0xFF, 0xFF}};
            driver.handle_ongoing_pressure_response(sensor_response);
            THEN(
                "no data is sent via the CAN bus, and the sync pin is "
                "set") {
                REQUIRE(i2c_queue.get_size() == 0);
                REQUIRE(can_queue.get_size() == 0);

                REQUIRE(hardware.get_sync_state_mock() == true);
            }
        }
    }

    GIVEN("output binding = report") {
        driver.set_bind_sync(true);
        std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                        sensors::utils::ResponseTag::IS_BASELINE,
                        sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
        auto tags_as_int = sensors::utils::byte_from_tags(tags);
        WHEN("a limited poll for 3 reads is set") {
            driver.poll_limited_pressure(3, tags_as_int);
            THEN("a pressure read poll") {
                REQUIRE(i2c_poll_queue.get_size() == 1);
                auto read_command =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        i2c_poll_queue);

                REQUIRE(
                    read_command.first.write_buffer[0] ==
                    static_cast<uint8_t>(
                        sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ));
                REQUIRE(read_command.polling == 3);
            }
        }
    }

    GIVEN("a baseline sensor request with 10 reads is processed") {
        driver.set_echoing(false);
        driver.set_bind_sync(false);

        std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                        sensors::utils::ResponseTag::IS_BASELINE,
                        sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
        auto tags_as_int = sensors::utils::byte_from_tags(tags);
        driver.poll_limited_pressure(10, tags_as_int);
        WHEN("pressure driver receives the requested sensor readings") {
            auto id = i2c::messages::TransactionIdentifier{
                .token = sensors::utils::build_id(
                    sensors::mmr920::ADDRESS,
                    static_cast<uint8_t>(
                        sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                    tags_as_int),
                .is_completed_poll = false,
                .transaction_index = static_cast<uint8_t>(0)};
            for (int i = 0; i < 5; i++) {
                auto sensor_response = i2c::messages::TransactionResponse{
                    .id = id,
                    .bytes_read = 3,
                    .read_buffer = {0x0, 0x54, 0x0, 0x00, 0x0, 0x0, 0x0, 0x0,
                                    0x0}};
                driver.handle_baseline_pressure_response(sensor_response);
            }
            for (int i = 0; i < 4; i++) {
                auto sensor_response = i2c::messages::TransactionResponse{
                    .id = id,
                    .bytes_read = 3,
                    .read_buffer = {0x00, 0x9B, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                    0x0}};
                driver.handle_baseline_pressure_response(sensor_response);
            }
            // complete the auto zero so the baseline message will be sent
            id.is_completed_poll = true;
            auto sensor_response = i2c::messages::TransactionResponse{
                .id = id,
                .bytes_read = 3,
                .read_buffer = {0x00, 0x9B, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};
            driver.handle_baseline_pressure_response(sensor_response);
            THEN(
                "a BaselineSensorResponse is sent with the correct calculated "
                "average") {
                can::message_writer_task::TaskMessage can_msg{};

                can_queue.try_read(&can_msg);
                auto response_msg =
                    std::get<can::messages::BaselineSensorResponse>(
                        can_msg.message);

                float check_data =
                    fixed_point_to_float(response_msg.offset_average, 16);
                float expected = 30.00048;
                REQUIRE(check_data == Approx(expected));
            }
        }
    }
}
