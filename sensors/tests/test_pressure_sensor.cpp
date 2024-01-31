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

constexpr auto sensor_id = can::ids::SensorId::S0;
constexpr uint8_t pressure_id =
    static_cast<uint8_t>(can::ids::SensorType::pressure);
constexpr uint8_t pressure_temperature_id =
    static_cast<uint8_t>(can::ids::SensorType::pressure_temperature);
constexpr uint8_t sensor_id_int = 0x0;

SCENARIO("Receiving messages through the pressure sensor message handler") {
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
        writer, poller, queue_client, response_queue, mock_hw, sensor_id,
        sensors::mmr920::SensorVersion::mmr920c04};

    GIVEN("A TransactionResponse message") {
        can_queue.reset();
        i2c::messages::TransactionResponse response_details{
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
            .read_buffer = {0x3D, 0x09, 0x00}};

        can::message_writer_task::TaskMessage empty_can_msg{};
        WHEN("the handler function receives a continuous poll response") {
            // prep for continuous read request
            auto bind_pressure = sensors::utils::TaskMessage(
                can::messages::BindSensorOutputRequest(
                    {}, 0xdeadbeef, can::ids::SensorType::pressure,
                    can::ids::SensorId::S0, 2));
            sensor.handle_message(bind_pressure);

            // discard the ack response
            can_queue.reset();
            std::array tags_for_continuous{
                sensors::utils::ResponseTag::IS_PART_OF_POLL,
                sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            response_details.id.token = sensors::utils::build_id(
                sensors::mmr920::ADDRESS,
                static_cast<uint8_t>(
                    sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                sensors::utils::byte_from_tags(tags_for_continuous));
            auto response_read = sensors::utils::TaskMessage(response_details);
            sensor.handle_message(response_read);
            THEN("there should be a ReadFromSensorResponse") {
                REQUIRE(can_queue.has_message());
                REQUIRE(can_queue.get_size() == 1);
                can_queue.try_read(&empty_can_msg);
                REQUIRE(std::holds_alternative<
                        can::messages::ReadFromSensorResponse>(
                    empty_can_msg.message));
            }
        }

        WHEN("the handler function receives a baseline poll response") {
            // prep for baseline sensor request
            auto single_read = sensors::utils::TaskMessage(
                can::messages::BaselineSensorRequest(
                    {}, 0xdeadbeef, pressure_id, sensor_id_int));
            sensor.handle_message(single_read);

            // discard the ack response
            can_queue.reset();
            std::array tags_for_baseline{
                sensors::utils::ResponseTag::IS_PART_OF_POLL,
                sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
            response_details.id.token = sensors::utils::build_id(
                sensors::mmr920::ADDRESS,
                static_cast<uint8_t>(
                    sensors::mmr920::Registers::LOW_PASS_PRESSURE_READ),
                sensors::utils::byte_from_tags(tags_for_baseline));
            auto response_read = sensors::utils::TaskMessage(response_details);
            sensor.handle_message(response_read);
            THEN("there should be a BaselineSensorResponse") {
                REQUIRE(can_queue.has_message());
                REQUIRE(can_queue.get_size() == 1);
                can_queue.try_read(&empty_can_msg);
                REQUIRE(std::holds_alternative<
                        can::messages::BaselineSensorResponse>(
                    empty_can_msg.message));
            }
        }

        WHEN("the handler function receives an unsupported register response") {
            std::array tags_for_baseline{
                sensors::utils::ResponseTag::IS_PART_OF_POLL,
                sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
            response_details.id.token = sensors::utils::build_id(
                sensors::mmr920::ADDRESS,
                static_cast<uint8_t>(sensors::mmr920::Registers::MACRAM_WRITE),
                sensors::utils::byte_from_tags(tags_for_baseline));
            auto response_read = sensors::utils::TaskMessage(response_details);
            sensor.handle_message(response_read);
            THEN("there should be no response from the driver") {
                REQUIRE(!can_queue.has_message());
            }
        }
    }
}
// TODO: implement and test polling for the pressure sensor
