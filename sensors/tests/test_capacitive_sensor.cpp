#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/core/logging.h"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_queue_client.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"
#include "motor-control/core/utils.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/utils.hpp"
#include "sensors/tests/mock_hardware.hpp"

template <typename Message, typename Queue>
auto get_message(Queue& q) -> Message {
    i2c::poller::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

SCENARIO("read capacitance sensor values") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};
    test_mocks::MockMessageQueue<sensors::utils::TaskMessage>
        capacitive_queue{};

    i2c::writer::TaskMessage empty_msg{};
    i2c::poller::TaskMessage empty_poll_msg{};

    test_mocks::MockI2CResponseQueue response_queue{};
    auto queue_client =
        mock_client::QueueClient{.capacitive_sensor_queue = &capacitive_queue};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    poller.set_queue(&poller_queue);

    auto sensor = sensors::tasks::CapacitiveMessageHandler{
        writer, poller, mock_hw, queue_client, response_queue};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a request to take a single read of the capacitive sensor") {
        auto single_read = sensors::utils::TaskMessage(
            can_messages::ReadFromSensorRequest({}, capacitive_id));
        sensor.handle_message(single_read);
        WHEN("the handler function receives the message") {
            THEN("the i2c poller queue is populated with a poll request") {
                REQUIRE(poller_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::MultiRegisterPollRead>(
                        poller_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            sensors::fdc1004::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            sensors::fdc1004::LSB_MEASUREMENT_1);
                }
                AND_WHEN("we send just one response") {
                    sensors::utils::TaskMessage first =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true));
                    sensor.handle_message(first);
                    THEN("no can response is sent") {
                        REQUIRE(!can_queue.has_message());
                    }
                }
                AND_WHEN("we send two responses") {
                    sensors::utils::TaskMessage first =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true));
                    sensor.handle_message(first);
                    sensors::utils::TaskMessage second =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             true));
                    sensor.handle_message(second);
                    THEN("after the second, a response is sent") {
                        REQUIRE(can_queue.has_message());
                    }
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the capacitance sensor") {
        int NUM_READS = 2;
        auto multi_read = sensors::utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, capacitive_id, NUM_READS));
        sensor.handle_message(multi_read);
        WHEN("the handler function receives the message in LSB mode") {
            THEN("the poller queue is populated with a poll request") {
                REQUIRE(poller_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::MultiRegisterPollRead>(
                        poller_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            sensors::fdc1004::MSB_MEASUREMENT_1);
                    REQUIRE(read_message.first.bytes_to_write == 1);
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            sensors::fdc1004::LSB_MEASUREMENT_1);
                    REQUIRE(read_message.second.bytes_to_write == 1);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                THEN(
                    "using the callback with +saturated data returns the "
                    "expected value") {
                    std::array<uint8_t, 5> buffer_a = {0x7f, 0xff, 0, 0, 0};
                    std::array<uint8_t, 5> buffer_b = {0xff, 0, 0, 0, 0};
                    std::array<sensors::utils::TaskMessage, 4> responses{
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             false, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             false, buffer_b)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             true, buffer_b))};
                    for (auto& response : responses) {
                        sensor.handle_message(response);
                    }
                    mock_message_writer::TaskMessage can_msg{};

                    REQUIRE(can_queue.get_size() == 1);
                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = signed_fixed_point_to_float(
                        response_msg.sensor_data, 15);
                    float expected = 15;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                    LOG("thing im testing is done");
                }
                THEN(
                    "using the callback with -saturated data returns the "
                    "expected value") {
                    std::array<uint8_t, 5> buffer_a = {0x80, 0x00, 0, 0, 0};
                    std::array<uint8_t, 5> buffer_b = {0x00, 0, 0, 0, 0};
                    std::array<sensors::utils::TaskMessage, 4> responses{
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             false, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             false, buffer_b)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             true, buffer_b))};
                    for (auto& response : responses) {
                        sensor.handle_message(response);
                    }

                    REQUIRE(can_queue.get_size() == 1);
                    mock_message_writer::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can_messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = signed_fixed_point_to_float(
                        response_msg.sensor_data, 15);
                    float expected = -15;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }

    GIVEN("a drifting capacitance reading") {
        int NUM_READS = 30;
        auto multi_read = sensors::utils::TaskMessage(
            can_messages::BaselineSensorRequest({}, capacitive_id, NUM_READS));
        WHEN("we call the capacitance handler") {
            sensor.handle_message(multi_read);
            std::array<uint8_t, 5> buffer_a = {200, 80, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {100, 10, 0, 0, 0};
            auto read_message =
                get_message<i2c::messages::MultiRegisterPollRead>(poller_queue);
            for (int i = 0; i < NUM_READS; i++) {
                sensors::utils::TaskMessage first_resp =
                    test_mocks::launder_response(
                        read_message, response_queue,
                        test_mocks::dummy_multi_response(
                            read_message, 0, (i == (NUM_READS - 1)), buffer_a));
                sensors::utils::TaskMessage second_resp =
                    test_mocks::launder_response(
                        read_message, response_queue,
                        test_mocks::dummy_multi_response(
                            read_message, 1, (i == (NUM_READS - 1)), buffer_b));
                sensor.handle_message(first_resp);
                sensor.handle_message(second_resp);
            }
            THEN("it should adjust the offset accordingly") {
                // check for the offset
                auto read = sensors::utils::TaskMessage(
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
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<mock_message_writer::TaskMessage> can_queue{};

    auto queue_client = mock_client::QueueClient{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    sensors::tasks::ReadCapacitanceCallback callback_host(queue_client, writer,
                                                          mock_hw);

    mock_message_writer::TaskMessage empty_msg{};

    GIVEN("a callback instance that is echoing and not bound") {
        callback_host.set_echoing(true);
        callback_host.set_bind_sync(false);

        WHEN("it receives data under its threshold") {
            std::array<uint8_t, 5> buffer_a = {0, 0, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {0, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            sensors::fdc1004::MSB_MEASUREMENT_1,
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = 0,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;
            callback_host.handle_ongoing_response(first);
            callback_host.handle_ongoing_response(second);

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
            std::array<uint8_t, 5> buffer_a = {0x7f, 0xff, 0, 0};
            std::array<uint8_t, 5> buffer_b = {0xff, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            sensors::fdc1004::MSB_MEASUREMENT_1,
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = true,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;

            callback_host.handle_ongoing_response(first);
            callback_host.handle_ongoing_response(second);

            THEN("it should forward the converted data via can") {
                can_queue.try_read(&empty_msg);
                auto sent = std::get<can_messages::ReadFromSensorResponse>(
                    empty_msg.message);
                REQUIRE(sent.sensor == can_ids::SensorType::capacitive);
                // we're just checking that the data is faithfully represented,
                // don't really care what it is
                REQUIRE(sent.sensor_data == convert_to_fixed_point(15, 15));
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
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            sensors::fdc1004::MSB_MEASUREMENT_1,
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = 0,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;

            callback_host.handle_ongoing_response(first);
            callback_host.handle_ongoing_response(second);

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
            std::array<uint8_t, 5> buffer_a = {0x7f, 0xff, 0, 0, 0};
            std::array<uint8_t, 5> buffer_b = {0xff, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            sensors::fdc1004::MSB_MEASUREMENT_1,
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = 0,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;
            callback_host.set_threshold(10);
            callback_host.handle_ongoing_response(first);
            callback_host.handle_ongoing_response(second);

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
