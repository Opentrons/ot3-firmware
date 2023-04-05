#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
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

template <typename Message, typename Queue>
auto get_message_i2c(Queue& q) -> Message {
    i2c::writer::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

auto sensor_id = can::ids::SensorId::S0;
constexpr uint8_t sensor_id_int = 0x0;

SCENARIO("read capacitance sensor values without shared CINs") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
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

    auto sensor_not_shared = sensors::tasks::CapacitiveMessageHandler{
        writer, poller, mock_hw, queue_client, response_queue, false};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a initialize sensor request") {
        sensor_not_shared.initialize();

        WHEN("the driver receives the message") {
            THEN("the i2c queue receives two register commands to initialize") {
                REQUIRE(i2c_queue.get_size() == 2);
            }
            AND_WHEN("we inspect the configuration messages") {
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);
                auto message_2 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                uint16_t expected_fdc_CIN1 = 0x580;
                uint32_t actual_fdc_CIN1 = 0;

                const auto* iter_1 =
                    message_2.transaction.write_buffer.cbegin() + 1;
                static_cast<void>(bit_utils::bytes_to_int(
                    iter_1, message_2.transaction.write_buffer.cend(),
                    actual_fdc_CIN1));
                THEN("We have FDC configurations for one CIN") {
                    REQUIRE(actual_fdc_CIN1 == expected_fdc_CIN1);
                }
                THEN("We set the configuration registers for one CIN") {
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS1));
                }
            }
        }
    }

    GIVEN("a request to take a single read of the capacitive sensor") {
        auto single_read =
            sensors::utils::TaskMessage(can::messages::ReadFromSensorRequest(
                {}, 0xdeadbeef, capacitive_id, sensor_id_int));
        sensor_not_shared.handle_message(single_read);
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
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB));
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_LSB));
                }
                AND_WHEN("we send just one response") {
                    sensors::utils::TaskMessage first =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true));
                    sensor_not_shared.handle_message(first);
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
                    sensor_not_shared.handle_message(first);
                    sensors::utils::TaskMessage second =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             true));
                    sensor_not_shared.handle_message(second);
                    THEN("after the second, a response is sent") {
                        REQUIRE(can_queue.has_message());
                    }
                }
            }
        }
    }
    GIVEN("a request to take a baseline reading of the capacitance sensor") {
        int NUM_READS = 2;
        auto multi_read =
            sensors::utils::TaskMessage(can::messages::BaselineSensorRequest(
                {}, 0xdeadbeef, capacitive_id, sensor_id_int, NUM_READS));
        sensor_not_shared.handle_message(multi_read);
        can_queue.reset();
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
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB));
                    REQUIRE(read_message.first.bytes_to_write == 1);
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_LSB));
                    REQUIRE(read_message.second.bytes_to_write == 1);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                THEN(
                    "using the callback with +saturated data returns the "
                    "expected value") {
                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0x7f, 0xff, 0, 0, 0};
                    auto buffer_b =
                        i2c::messages::MaxMessageBuffer{0xff, 0, 0, 0, 0};
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
                        sensor_not_shared.handle_message(response);
                    }
                    can::message_writer_task::TaskMessage can_msg{};

                    REQUIRE(can_queue.get_size() == 1);
                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = signed_fixed_point_to_float(
                        response_msg.sensor_data, S15Q16_RADIX);
                    float expected = 15;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
                THEN(
                    "using the callback with -saturated data returns the "
                    "expected value") {
                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0x80, 0x00, 0, 0, 0};
                    auto buffer_b =
                        i2c::messages::MaxMessageBuffer{0x00, 0, 0, 0, 0};
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
                        sensor_not_shared.handle_message(response);
                    }

                    REQUIRE(can_queue.get_size() == 1);
                    can::message_writer_task::TaskMessage can_msg{};

                    can_queue.try_read(&can_msg);
                    auto response_msg =
                        std::get<can::messages::ReadFromSensorResponse>(
                            can_msg.message);
                    float check_data = signed_fixed_point_to_float(
                        response_msg.sensor_data, S15Q16_RADIX);
                    float expected = -15;
                    REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                }
            }
        }
    }

    GIVEN("a drifting capacitance reading") {
        int NUM_READS = 30;
        auto multi_read =
            sensors::utils::TaskMessage(can::messages::BaselineSensorRequest(
                {}, 0xdeadbeef, capacitive_id, sensor_id_int, NUM_READS));
        WHEN("we call the capacitance handler") {
            sensor_not_shared.handle_message(multi_read);
            can_queue.reset();
            auto buffer_a = i2c::messages::MaxMessageBuffer{200, 80, 0, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{100, 10, 0, 0, 0};
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
                sensor_not_shared.handle_message(first_resp);
                sensor_not_shared.handle_message(second_resp);
            }
            THEN("it should adjust the offset accordingly") {
                // check for the offset
                auto read = sensors::utils::TaskMessage(
                    can::messages::ReadFromSensorRequest(
                        {}, 0xdeadbeef, capacitive_id, sensor_id_int, 1));
                sensor_not_shared.handle_message(read);
                can::message_writer_task::TaskMessage can_msg{};

                can_queue.try_read(&can_msg);
                auto response_msg =
                    std::get<can::messages::ReadFromSensorResponse>(
                        can_msg.message);
                float expected = 0.3418;
                float check_data = fixed_point_to_float(
                    response_msg.sensor_data, S15Q16_RADIX);
                REQUIRE(check_data >= Approx(expected).epsilon(1e-4));
            }
        }
    }
}

SCENARIO("read capacitance sensor values supporting shared CINs") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
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

    auto sensor_shared = sensors::tasks::CapacitiveMessageHandler{
        writer, poller, mock_hw, queue_client, response_queue, true};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a initialize sensor request") {
        sensor_shared.initialize();

        WHEN("the driver receives the message") {
            THEN(
                "the i2c queue receives four register commands to initialize") {
                REQUIRE(i2c_queue.get_size() == 4);
            }
            AND_WHEN("we inspect the configuration messages") {
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);
                auto message_2 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);
                auto message_3 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);
                auto message_4 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                uint16_t expected_fdc_CIN1 = 0x580;
                uint16_t expected_fdc_CIN2 = 0x540;
                uint32_t actual_fdc_CIN1 = 0;
                uint32_t actual_fdc_CIN2 = 0;

                const auto* iter_1 =
                    message_2.transaction.write_buffer.cbegin() + 1;
                const auto* iter_2 =
                    message_4.transaction.write_buffer.cbegin() + 1;
                static_cast<void>(bit_utils::bytes_to_int(
                    iter_1, message_2.transaction.write_buffer.cend(),
                    actual_fdc_CIN1));
                static_cast<void>(bit_utils::bytes_to_int(
                    iter_2, message_4.transaction.write_buffer.cend(),
                    actual_fdc_CIN2));
                THEN("We have FDC configurations for two CINs") {
                    REQUIRE(actual_fdc_CIN1 == expected_fdc_CIN1);
                    REQUIRE(actual_fdc_CIN2 == expected_fdc_CIN2);
                }
                THEN("We set the configuration registers for both CINs") {
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS1));
                    REQUIRE(message_3.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS2));
                }
            }
        }
    }

    GIVEN("a series of reads with different sensor ids") {
        auto single_read_s1 =
            sensors::utils::TaskMessage(can::messages::ReadFromSensorRequest(
                {}, 0xdeadbeef, capacitive_id, 0x1));
        sensor_shared.handle_message(single_read_s1);
        WHEN("the handler function receives the message") {
            THEN("the i2c poller queue is populated with a poll request") {
                REQUIRE(poller_queue.get_size() == 1);
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::MultiRegisterPollRead>(
                        poller_queue);
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS2_MSB));
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS2_LSB));
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                }
                AND_WHEN(
                    "the handler function receives a second request for sensor "
                    "id S1") {
                    sensor_shared.handle_message(single_read_s1);
                    THEN(
                        "the i2c queue does not receive a request to update "
                        "the fdc register") {
                        REQUIRE(i2c_queue.get_size() == 0);
                    }
                }
            }
        }
        WHEN("a single read is sent for S0 after a read for s1") {
            auto single_read_s0 = sensors::utils::TaskMessage(
                can::messages::ReadFromSensorRequest({}, 0xdeadbeef,
                                                     capacitive_id, 0x0));
            sensor_shared.handle_message(single_read_s0);

            // throw out previous S1 call
            static_cast<void>(get_message<i2c::messages::MultiRegisterPollRead>(
                poller_queue));
            static_cast<void>(
                get_message_i2c<i2c::messages::Transact>(i2c_queue));

            THEN(
                "the i2c poller queue is populated with a poll request and the "
                "fdc register is updated") {
                REQUIRE(poller_queue.get_size() == 1);
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::MultiRegisterPollRead>(
                        poller_queue);
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB));
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_LSB));
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                }
                AND_WHEN(
                    "the handler function receives a second request for sensor "
                    "id S0") {
                    sensor_shared.handle_message(single_read_s0);
                    THEN(
                        "the i2c queue does not receive a request to update "
                        "the fdc register") {
                        REQUIRE(i2c_queue.get_size() == 0);
                    }
                }
            }
        }
    }
}

SCENARIO("capacitance driver tests no shared CINs") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};

    auto queue_client = mock_client::QueueClient{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};

    test_mocks::MockI2CResponseQueue response_queue{};

    queue_client.set_queue(&can_queue);
    writer.set_queue(&i2c_queue);
    sensors::tasks::FDC1004 callback_host(writer, poller, queue_client,
                                          response_queue, mock_hw, false);

    can::message_writer_task::TaskMessage empty_msg{};

    GIVEN("a callback instance that is echoing and not bound") {
        callback_host.set_echoing(true);
        callback_host.set_bind_sync(false);
        THEN("it should reset the sync line") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            REQUIRE(mock_hw.get_sync_reset_calls() == 1);
        }
        WHEN("it receives data under its threshold") {
            auto buffer_a = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB),
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
                REQUIRE(mock_hw.get_sync_reset_calls() == 1);
            }
        }
        WHEN("it receives data over its threshold") {
            auto buffer_a = i2c::messages::MaxMessageBuffer{0x7f, 0xff, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{0xff, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB),
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
                auto sent = std::get<can::messages::ReadFromSensorResponse>(
                    empty_msg.message);
                REQUIRE(sent.sensor == can::ids::SensorType::capacitive);
                // we're just checking that the data is faithfully represented,
                // don't really care what it is
                REQUIRE(sent.sensor_data ==
                        convert_to_fixed_point(15, S15Q16_RADIX));
            }
            THEN("it should not touch the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == false);
                REQUIRE(mock_hw.get_sync_set_calls() == 0);
                REQUIRE(mock_hw.get_sync_reset_calls() == 1);
            }
        }
    }
    GIVEN("a callback instance that is bound and not echoing") {
        callback_host.set_echoing(false);
        callback_host.set_bind_sync(true);
        THEN("it should reset its sync line") {
            REQUIRE(mock_hw.get_sync_reset_calls() == 1);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }

        WHEN("it receives data under its threshold") {
            auto buffer_a = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB),
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
                REQUIRE(mock_hw.get_sync_reset_calls() == 2);
            }
        }
        WHEN("it receives data over its threshold") {
            auto buffer_a =
                i2c::messages::MaxMessageBuffer{0x7f, 0xff, 0, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{0xff, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL,
                            sensors::utils::ResponseTag::POLL_IS_CONTINUOUS};
            i2c::messages::TransactionResponse first{
                .message_index = 1234,
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB),
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = 0,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;
            callback_host.set_threshold(10,
                                        can::ids::SensorThresholdMode::absolute,
                                        first.message_index);
            can_queue.reset();
            callback_host.handle_ongoing_response(first);
            callback_host.handle_ongoing_response(second);
            THEN("it should not send can messages") {
                REQUIRE(!can_queue.has_message());
            }
            THEN("it should assert the sync line") {
                REQUIRE(mock_hw.get_sync_state_mock() == true);
                REQUIRE(mock_hw.get_sync_set_calls() == 1);
                // this call is still the one from setting up
                // when we started the bind
                REQUIRE(mock_hw.get_sync_reset_calls() == 1);
            }
        }
    }
}

SCENARIO("threshold configuration") {
    test_mocks::MockSensorHardware mock_hw{};
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poller_queue{};

    test_mocks::MockMessageQueue<can::message_writer_task::TaskMessage>
        can_queue{};
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
        writer, poller, mock_hw, queue_client, response_queue, false};

    GIVEN("A request to set an autothreshold") {
        int NUM_READS = 10;
        auto autothreshold = sensors::utils::TaskMessage(
            can::messages::SetSensorThresholdRequest(
                {}, 0xdeadbeef, can::ids::SensorType::capacitive, sensor_id,
                convert_to_fixed_point(0.375, S15Q16_RADIX),
                can::ids::SensorThresholdMode::auto_baseline));
        WHEN("the message is received") {
            sensor.handle_message(autothreshold);
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
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_MSB));
                    REQUIRE(read_message.first.bytes_to_write == 1);
                    REQUIRE(read_message.second.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.second.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS1_LSB));
                    REQUIRE(read_message.second.bytes_to_write == 1);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                AND_WHEN("using the callback with data") {
                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0x7f, 0xff, 0, 0, 0};
                    auto buffer_b =
                        i2c::messages::MaxMessageBuffer{0xff, 0, 0, 0, 0};
                    for (int i = 0; i < NUM_READS - 1; i++) {
                        auto response_a = sensors::utils::TaskMessage(
                            test_mocks::launder_response(
                                read_message, response_queue,
                                test_mocks::dummy_multi_response(
                                    read_message, 0, false, buffer_a)));
                        sensor.handle_message(response_a);
                        auto response_b = sensors::utils::TaskMessage(
                            test_mocks::launder_response(
                                read_message, response_queue,
                                test_mocks::dummy_multi_response(
                                    read_message, 1, false, buffer_b)));
                        sensor.handle_message(response_b);
                    }
                    auto final_response_a = sensors::utils::TaskMessage(
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 0,
                                                             true, buffer_a)));
                    auto final_response_b = sensors::utils::TaskMessage(
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_multi_response(read_message, 1,
                                                             true, buffer_b)));
                    sensor.handle_message(final_response_a);
                    sensor.handle_message(final_response_b);
                    THEN("the threshold is set to the proper value") {
                        REQUIRE(sensor.driver.get_threshold() ==
                                Approx(15.375).epsilon(1e-4));
                    }
                    THEN(
                        "a message is sent on can informing that the threshold "
                        "is set") {
                        REQUIRE(can_queue.get_size() == 1);
                        can::message_writer_task::TaskMessage can_msg{};
                        can_queue.try_read(&can_msg);

                        auto response_msg =
                            std::get<can::messages::SensorThresholdResponse>(
                                can_msg.message);
                        float check_data = signed_fixed_point_to_float(
                            response_msg.threshold, S15Q16_RADIX);
                        // the average value + 1
                        float expected = 15.375;
                        REQUIRE(check_data == Approx(expected).epsilon(1e-4));
                        REQUIRE(response_msg.mode ==
                                can::ids::SensorThresholdMode::auto_baseline);
                    }
                }
            }
        }
    }

    GIVEN("A request to set a specific threshold") {
        auto specific_threshold = sensors::utils::TaskMessage(
            can::messages::SetSensorThresholdRequest(
                {}, 0xdeadbeef, can::ids::SensorType::capacitive, sensor_id,
                convert_to_fixed_point(10, S15Q16_RADIX),
                can::ids::SensorThresholdMode::absolute));
        WHEN("the message is received") {
            sensor.handle_message(specific_threshold);

            THEN("the sensor should send a response with the same threshold") {
                REQUIRE(can_queue.get_size() == 1);
                can::message_writer_task::TaskMessage can_response{};
                REQUIRE(can_queue.try_read(&can_response) == true);
                auto threshold_response =
                    std::get<can::messages::SensorThresholdResponse>(
                        can_response.message);
                REQUIRE(threshold_response.sensor ==
                        can::ids::SensorType::capacitive);
                REQUIRE(threshold_response.threshold ==
                        convert_to_fixed_point(10, S15Q16_RADIX));
                REQUIRE(threshold_response.mode ==
                        can::ids::SensorThresholdMode::absolute);
            }
            THEN("the sensor's threshold should be set") {
                REQUIRE(sensor.driver.get_threshold() == 10);
            }
        }
    }
}
