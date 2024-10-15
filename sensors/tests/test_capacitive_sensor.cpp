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

static std::array<float, SENSOR_BUFFER_SIZE> sensor_buffer;

SCENARIO("read capacitance sensor values without shared CINs") {
    auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw(version_wrapper, sync_control);
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
        writer,         poller, mock_hw,       queue_client,
        response_queue, false,  &sensor_buffer};
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

                uint16_t expected_fdc_CIN1 = 0x5C0;
                uint16_t actual_fdc_CIN1 = 0;

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
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        poller_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                }
                AND_WHEN("we send just one response") {
                    sensors::utils::TaskMessage first =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_single_response(read_message,
                                                              true));
                    sensor_not_shared.handle_message(first);
                    THEN("no can response is sent") {
                        REQUIRE(!can_queue.has_message());
                    }
                }
                AND_WHEN("we send the full responses") {
                    i2c::messages::MaxMessageBuffer fdc_resp = {0x00, 0xFF};
                    sensors::utils::TaskMessage fdc =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_single_response(read_message,
                                                              true, fdc_resp));
                    sensor_not_shared.handle_message(fdc);
                    REQUIRE(i2c_queue.get_size() == 1);
                    auto msb_message =
                        get_message_i2c<i2c::messages::Transact>(i2c_queue);
                    msb_message.id.is_completed_poll = true;
                    sensors::utils::TaskMessage first =
                        test_mocks::launder_response(
                            msb_message, response_queue,
                            test_mocks::dummy_response(msb_message));
                    sensor_not_shared.handle_message(first);
                    REQUIRE(i2c_queue.get_size() == 1);
                    auto lsb_message =
                        get_message_i2c<i2c::messages::Transact>(i2c_queue);
                    lsb_message.id.is_completed_poll = true;
                    sensors::utils::TaskMessage second =
                        test_mocks::launder_response(
                            lsb_message, response_queue,
                            test_mocks::dummy_response(lsb_message));
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
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        poller_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                    REQUIRE(read_message.first.bytes_to_write == 1);
                    REQUIRE(read_message.first.bytes_to_read == 2);
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

                    i2c::messages::MaxMessageBuffer fdc_resp = {0x00, 0xFF};
                    sensors::utils::TaskMessage fdc =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_single_response(read_message,
                                                              true, fdc_resp));
                    sensor_not_shared.handle_message(fdc);
                    auto msb_message =
                        get_message_i2c<i2c::messages::Transact>(i2c_queue);

                    std::array<i2c::messages::TransactionResponse, 4> responses{
                        test_mocks::launder_response(
                            msb_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_b)),
                        test_mocks::launder_response(
                            msb_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_b))};

                    responses[0].id.is_completed_poll = false;
                    responses[1].id.is_completed_poll = false;
                    responses[1].id.transaction_index = 1;
                    responses[2].id.is_completed_poll = true;
                    responses[3].id.transaction_index = 1;
                    responses[3].id.is_completed_poll = true;

                    for (auto& response : responses) {
                        sensors::utils::TaskMessage msg{response};
                        sensor_not_shared.handle_message(msg);
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
                    i2c::messages::MaxMessageBuffer fdc_resp = {0x00, 0xFF};
                    sensors::utils::TaskMessage fdc =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_single_response(read_message,
                                                              true, fdc_resp));
                    sensor_not_shared.handle_message(fdc);
                    auto msb_message =
                        get_message_i2c<i2c::messages::Transact>(i2c_queue);

                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0x80, 0x00, 0, 0, 0};
                    auto buffer_b =
                        i2c::messages::MaxMessageBuffer{0x00, 0, 0, 0, 0};
                    std::array<i2c::messages::TransactionResponse, 4> responses{
                        test_mocks::launder_response(
                            msb_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_b)),
                        test_mocks::launder_response(
                            msb_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_a)),
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_response(msb_message, buffer_b))};

                    responses[0].id.is_completed_poll = false;
                    responses[1].id.is_completed_poll = false;
                    responses[1].id.transaction_index = 1;
                    responses[2].id.is_completed_poll = true;
                    responses[3].id.transaction_index = 1;
                    responses[3].id.is_completed_poll = true;

                    for (auto& response : responses) {
                        sensors::utils::TaskMessage msg{response};
                        sensor_not_shared.handle_message(msg);
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
                get_message<i2c::messages::SingleRegisterPollRead>(
                    poller_queue);

            i2c::messages::MaxMessageBuffer fdc_resp = {0x00, 0xFF};
            sensors::utils::TaskMessage fdc =
                test_mocks::launder_response(read_message, response_queue,
                                             test_mocks::dummy_single_response(
                                                 read_message, true, fdc_resp));
            sensor_not_shared.handle_message(fdc);
            REQUIRE(i2c_queue.get_size() == 1);
            auto msb_message =
                get_message_i2c<i2c::messages::Transact>(i2c_queue);

            for (int i = 0; i < NUM_READS; i++) {
                i2c::messages::TransactionResponse first_resp =
                    test_mocks::launder_response(
                        msb_message, response_queue,
                        test_mocks::dummy_response(msb_message, buffer_a));
                i2c::messages::TransactionResponse second_resp =
                    test_mocks::launder_response(
                        msb_message, response_queue,
                        test_mocks::dummy_response(msb_message, buffer_b));
                first_resp.id.transaction_index = 0;
                second_resp.id.transaction_index = 1;
                first_resp.id.is_completed_poll = (i == (NUM_READS - 1));
                second_resp.id.is_completed_poll = (i == (NUM_READS - 1));
                auto first_msg = sensors::utils::TaskMessage{first_resp};
                auto second_msg = sensors::utils::TaskMessage{second_resp};
                sensor_not_shared.handle_message(first_msg);
                sensor_not_shared.handle_message(second_msg);
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
    auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw{version_wrapper, sync_control};
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
        writer,         poller, mock_hw,       queue_client,
        response_queue, true,   &sensor_buffer};
    constexpr uint8_t capacitive_id = 0x1;

    GIVEN("a initialize sensor request") {
        sensor_shared.initialize();

        WHEN("the driver receives the message") {
            THEN("the i2c queue receives two register commands to initialize") {
                REQUIRE(i2c_queue.get_size() == 2);
            }
            AND_WHEN("we inspect the configuration messages") {
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);
                auto message_2 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                uint16_t expected_fdc_CIN = 0x5C0;
                uint16_t actual_fdc_CIN = 0;

                const auto* iter_1 =
                    message_2.transaction.write_buffer.cbegin() + 1;
                static_cast<void>(bit_utils::bytes_to_int(
                    iter_1, message_2.transaction.write_buffer.cend(),
                    actual_fdc_CIN));
                THEN("We have FDC configurations for both CINs") {
                    REQUIRE(actual_fdc_CIN == expected_fdc_CIN);
                }
                THEN("We set the configuration registers for the first CIN") {
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS1));
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
            THEN(
                "the i2c poller queue is populated with a poll request and one "
                "i2c write") {
                REQUIRE(poller_queue.get_size() == 1);
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        poller_queue);
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS2));
                }
                AND_WHEN("the FDC response is read") {
                    i2c::messages::MaxMessageBuffer fdc_resp = {0x00, 0xFF};
                    sensors::utils::TaskMessage fdc =
                        test_mocks::launder_response(
                            read_message, response_queue,
                            test_mocks::dummy_single_response(read_message,
                                                              true, fdc_resp));
                    sensor_shared.handle_message(fdc);
                    THEN("a request to read CH2 MSB is sent") {
                        REQUIRE(i2c_queue.get_size() == 1);
                        auto message_msb =
                            get_message_i2c<i2c::messages::Transact>(i2c_queue);
                        REQUIRE(message_msb.transaction.write_buffer[0] ==
                                static_cast<uint8_t>(
                                    sensors::fdc1004::Registers::MEAS2_MSB));
                    }
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
            static_cast<void>(
                get_message<i2c::messages::SingleRegisterPollRead>(
                    poller_queue));
            static_cast<void>(
                get_message_i2c<i2c::messages::Transact>(i2c_queue));

            THEN(
                "the i2c poller queue is populated with a poll request and the "
                "CONF_MEAS1 register is updated") {
                REQUIRE(poller_queue.get_size() == 1);
                REQUIRE(i2c_queue.get_size() == 1);
            }
            AND_WHEN("we read the messages from the queue") {
                auto read_message =
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        poller_queue);
                auto message_1 =
                    get_message_i2c<i2c::messages::Transact>(i2c_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                    REQUIRE(message_1.transaction.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::CONF_MEAS1));
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

    GIVEN("some transaction response messages") {
        sensor_shared.driver.set_sensor_id(can::ids::SensorId::S1);
        sensor_shared.driver.set_echoing(true);
        sensor_shared.driver.set_bind_sync(false);

        WHEN("A response for S1 is received") {
            auto buffer_a = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            auto buffer_b = i2c::messages::MaxMessageBuffer{0, 0, 0, 0, 0};
            std::array tags{sensors::utils::ResponseTag::IS_PART_OF_POLL};
            i2c::messages::TransactionResponse first{
                .id =
                    i2c::messages::TransactionIdentifier{
                        .token = sensors::utils::build_id(
                            sensors::fdc1004::ADDRESS,
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::MEAS2_MSB),
                            sensors::utils::byte_from_tags(tags)),
                        .is_completed_poll = 0,
                        .transaction_index = 0},
                .bytes_read = 2,
                .read_buffer = buffer_a};
            auto second = first;
            second.id.transaction_index = 1;
            second.read_buffer = buffer_b;
            second.id.is_completed_poll=1;
            auto first_task_msg = sensors::utils::TaskMessage(first);
            auto second_task_msg = sensors::utils::TaskMessage(second);
            sensor_shared.handle_message(first_task_msg);
            sensor_shared.handle_message(second_task_msg);

            THEN("it should forward the converted data via can") {
                REQUIRE(can_queue.has_message());
            }
        }
    }
}

SCENARIO("capacitance driver tests no shared CINs") {
    auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw{version_wrapper, sync_control};
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
                                          response_queue, mock_hw, false,
                                          &sensor_buffer);

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
            for (auto i = 0; i < 14; i++) {
                callback_host.handle_ongoing_response(first);
                callback_host.handle_ongoing_response(second);
            }

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

            for (auto i = 0; i < 14; i++) {
                callback_host.handle_ongoing_response(first);
                callback_host.handle_ongoing_response(second);
            }

            THEN("it should forward the converted data via can") {
                can_queue.try_read(&empty_msg);
                auto sent =
                    std::get<can::messages::BatchReadFromSensorResponse>(
                        empty_msg.message);
                REQUIRE(sent.sensor == can::ids::SensorType::capacitive);
                // we're just checking that the data is faithfully represented,
                // don't really care what it is
                REQUIRE(sent.sensor_data[0] ==
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
    auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw{version_wrapper, sync_control};
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
        writer,         poller, mock_hw,       queue_client,
        response_queue, false,  &sensor_buffer};

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
                    get_message<i2c::messages::SingleRegisterPollRead>(
                        poller_queue);

                THEN("The write and read command addresses are correct") {
                    REQUIRE(read_message.first.address ==
                            sensors::fdc1004::ADDRESS);
                    REQUIRE(read_message.first.write_buffer[0] ==
                            static_cast<uint8_t>(
                                sensors::fdc1004::Registers::FDC_CONF));
                    REQUIRE(read_message.first.bytes_to_write == 1);
                    REQUIRE(read_message.delay_ms == 20);
                    REQUIRE(read_message.polling == NUM_READS);
                }
                AND_WHEN("using the callback with data") {
                    auto buffer_a =
                        i2c::messages::MaxMessageBuffer{0x7f, 0xff, 0, 0, 0};
                    auto buffer_b =
                        i2c::messages::MaxMessageBuffer{0xff, 0, 0, 0, 0};
                    for (int i = 0; i < NUM_READS; i++) {
                        std::array tags{
                            sensors::utils::ResponseTag::IS_THRESHOLD_SENSE};
                        i2c::messages::TransactionResponse first{
                            .id =
                                i2c::messages::TransactionIdentifier{
                                    .token = sensors::utils::build_id(
                                        sensors::fdc1004::ADDRESS,
                                        static_cast<uint8_t>(
                                            sensors::fdc1004::Registers::
                                                MEAS1_MSB),
                                        sensors::utils::byte_from_tags(tags)),
                                    .is_completed_poll = 0,
                                    .transaction_index = 0},
                            .bytes_read = 2,
                            .read_buffer = buffer_a};
                        if (i == (NUM_READS - 1)) {
                            first.id.is_completed_poll = true;
                        }
                        auto second = first;
                        second.id.transaction_index = 1;
                        second.read_buffer = buffer_b;
                        auto response_a = sensors::utils::TaskMessage(first);
                        sensor.handle_message(response_a);
                        auto response_b = sensors::utils::TaskMessage(second);
                        sensor.handle_message(response_b);
                    }
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
