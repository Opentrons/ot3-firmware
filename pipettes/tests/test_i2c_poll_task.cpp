#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_timer.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/messages.hpp"
#include "pipettes/core/tasks/i2c_poll_task.hpp"

SCENARIO("test the limited-count i2c poller") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};

    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    auto poll_handler = i2c_poller_task::I2CPollingMessageHandler<
        test_mocks::MockMessageQueue, test_mocks::MockTimer>{writer};

    GIVEN("single register limited poll command") {
        uint16_t addr = 0x1234;
        std::array<uint8_t, 5> register_payload{0x2, 0x3, 0x4, 0x5, 0x6};
        uint32_t handle_buff_call_count = 0;
        std::array<uint8_t, 5> last_handle_buff_value{};
        uint32_t client_cb_call_count = 0;
        auto handle_buf = [&handle_buff_call_count, &last_handle_buff_value](
                              const std::array<uint8_t, 5>& buf) {
            handle_buff_call_count++;
            last_handle_buff_value = buf;
        };
        auto client_cb = [&client_cb_call_count]() { client_cb_call_count++; };
        int poll_count = 10;
        int delay = 100;
        pipette_messages::SingleRegisterPollReadFromI2C msg{
            .address = addr,
            .polling = poll_count,
            .delay_ms = delay,
            .buffer = register_payload,
            .client_callback = client_cb,
            .handle_buffer = handle_buf};
        auto tm = i2c_poller::TaskMessage{msg};
        WHEN("executing the command") {
            poll_handler.handle_message(tm);
            auto& limited_polls = poll_handler.limited_polls;
            auto& poll = limited_polls.polls[0];

            THEN("nothing is immediately enqueued") {
                REQUIRE(!i2c_queue.has_message());
            }
            THEN("a poll object is provisioned with a timer") {
                REQUIRE(poll.address == addr);
                REQUIRE(poll.timer.get_period() ==
                        static_cast<uint32_t>(delay));
                REQUIRE(poll.timer.is_running());
            }
            THEN("only one poll object is provisioned") {
                auto poll_iter = limited_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != limited_polls.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                }
            }
            AND_WHEN("firing the timer once") {
                poll.timer.fire();
                THEN("a correct transaction is sent to the i2c task") {
                    REQUIRE(i2c_queue.get_size() == 1);
                    i2c_writer::TaskMessage empty_msg{};
                    i2c_queue.try_read(&empty_msg);
                    auto transaction =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    REQUIRE(transaction.buffer == register_payload);
                    REQUIRE(transaction.address == addr);
                    AND_WHEN("that transaction's buffer is handled") {
                        std::array<uint8_t, 5> response = {0xf, 0xe, 0xd, 0xc,
                                                           0xb};
                        transaction.handle_buffer(response);
                        THEN("the upstream buffer handler is called") {
                            REQUIRE(handle_buff_call_count == 1);
                            REQUIRE(client_cb_call_count == 0);
                            REQUIRE(last_handle_buff_value == response);
                        }
                    }
                    AND_WHEN("that transaction's completer is handled") {
                        transaction.client_callback();
                        THEN(
                            "the upstream completer is not immediately "
                            "called") {
                            REQUIRE(client_cb_call_count == 0);
                        }
                    }
                }
            }
            AND_WHEN("firing the timer poll_count-1 times") {
                i2c_writer::TaskMessage empty_msg;
                std::array<uint8_t, 5> response{0xaa, 0xbb, 0xcc, 0xdd, 0xee};
                for (int count = 0; count < (poll_count - 1); count++) {
                    poll.timer.fire();
                    CHECK(i2c_queue.get_size() == 1);
                    i2c_queue.try_read(&empty_msg);
                    auto transaction =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    transaction.handle_buffer(response);
                    transaction.client_callback();
                }
                THEN("the client callback still has not been called") {
                    REQUIRE(client_cb_call_count == 0);
                }
                THEN("the buffer callback has been called each time") {
                    REQUIRE(handle_buff_call_count ==
                            static_cast<uint32_t>(poll_count - 1));
                }
                AND_WHEN("firing the last poll") {
                    poll.timer.fire();
                    i2c_queue.try_read(&empty_msg);
                    auto last_txn =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    std::array<uint8_t, 5> response = {0xf, 0xe, 0xd, 0xc, 0xb};
                    last_txn.handle_buffer(response);
                    last_txn.client_callback();
                    THEN("the upstream client callback is called") {
                        REQUIRE(client_cb_call_count == 1);
                    }
                    THEN("the poll is no longer active") {
                        REQUIRE(!poll.timer.is_running());
                        REQUIRE(poll.address == 0);
                    }
                }
            }
        }
    }
    GIVEN("multi register limited poll command") {
        uint16_t addr = 0x1234;
        std::array<uint8_t, 5> register_payload_1{0x2, 0x3, 0x4, 0x5, 0x6};
        std::array<uint8_t, 5> register_payload_2{0x4, 0x6, 0x8, 0xa, 0xc};
        uint32_t handle_buff_call_count = 0;
        std::array<uint8_t, 5> last_handle_buff_value_1{};
        std::array<uint8_t, 5> last_handle_buff_value_2{};
        uint32_t client_cb_call_count = 0;
        auto handle_buf = [&handle_buff_call_count, &last_handle_buff_value_1,
                           &last_handle_buff_value_2](
                              const std::array<uint8_t, 5>& buf_1,
                              const std::array<uint8_t, 5>& buf_2) {
            handle_buff_call_count++;
            last_handle_buff_value_1 = buf_1;
            last_handle_buff_value_2 = buf_2;
        };
        auto client_cb = [&client_cb_call_count]() { client_cb_call_count++; };
        int poll_count = 10;
        int delay = 100;
        pipette_messages::MultiRegisterPollReadFromI2C msg{
            .address = addr,
            .polling = poll_count,
            .delay_ms = delay,
            .register_1_buffer = register_payload_1,
            .register_2_buffer = register_payload_2,
            .client_callback = client_cb,
            .handle_buffer = handle_buf};
        auto tm = i2c_poller::TaskMessage{msg};
        WHEN("executing the command") {
            poll_handler.handle_message(tm);
            auto& limited_polls = poll_handler.limited_polls;
            auto& poll = limited_polls.polls[0];

            THEN("nothing is immediately enqueued") {
                REQUIRE(!i2c_queue.has_message());
            }
            THEN("a poll object is provisioned with a timer") {
                REQUIRE(poll.address == addr);
                REQUIRE(poll.timer.get_period() ==
                        static_cast<uint32_t>(delay));
                REQUIRE(poll.timer.is_running());
            }
            THEN("only one poll object is provisioned") {
                auto poll_iter = limited_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != limited_polls.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                }
            }
            AND_WHEN("firing the timer once") {
                poll.timer.fire();
                THEN("a correct transaction is sent to the i2c task") {
                    REQUIRE(i2c_queue.get_size() == 1);
                    i2c_writer::TaskMessage empty_msg{};
                    i2c_queue.try_read(&empty_msg);
                    auto transaction =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    REQUIRE(transaction.buffer == register_payload_1);
                    REQUIRE(transaction.address == addr);
                    AND_WHEN("that transaction's buffer is handled") {
                        std::array<uint8_t, 5> response_1 = {0xf, 0xe, 0xd, 0xc,
                                                             0xb};
                        transaction.handle_buffer(response_1);
                        THEN("the upstream buffer handler is not yet called") {
                            REQUIRE(handle_buff_call_count == 0);
                        }
                        THEN("another transaction is enqueued") {
                            REQUIRE(i2c_queue.has_message());
                            i2c_queue.try_read(&empty_msg);
                            auto transaction_2 =
                                std::get<pipette_messages::TransactWithI2C>(
                                    empty_msg);
                            AND_WHEN("that second transaction is handled") {
                                std::array<uint8_t, 5> response_2 = {
                                    0xff, 0xee, 0xdd, 0xcc, 0xbb};
                                transaction_2.handle_buffer(response_2);
                                THEN(
                                    "the upstream handle buffer is finally "
                                    "called") {
                                    REQUIRE(handle_buff_call_count == 1);
                                    REQUIRE(last_handle_buff_value_1 ==
                                            response_1);
                                    REQUIRE(last_handle_buff_value_2 ==
                                            response_2);
                                }
                                THEN(
                                    "the upstream completer is not immediately "
                                    "called") {
                                    REQUIRE(client_cb_call_count == 0);
                                }
                            }
                        }
                    }
                }
            }
            AND_WHEN("firing the timer enough times to exhaust the poll") {
                i2c_writer::TaskMessage empty_msg{};
                std::array<uint8_t, 5> first_response = {0xf, 0xe, 0xd, 0xc,
                                                         0xb};
                std::array<uint8_t, 5> second_response = {0xff, 0xee, 0xdd,
                                                          0xcc, 0xbb};
                for (int count = 0; count < (poll_count - 1); count++) {
                    poll.timer.fire();
                    CHECK(i2c_queue.get_size() == 1);
                    i2c_queue.try_read(&empty_msg);
                    auto txn =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    txn.handle_buffer(first_response);
                    txn.client_callback();
                    CHECK(i2c_queue.get_size() == 1);
                    i2c_queue.try_read(&empty_msg);
                    txn =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    txn.handle_buffer(second_response);
                    txn.client_callback();
                    CHECK(i2c_queue.get_size() == 0);
                }
                THEN(
                    "the correct number of transactions were sent to the i2c "
                    "task") {
                    REQUIRE(handle_buff_call_count ==
                            static_cast<uint32_t>(poll_count - 1));
                }
                AND_WHEN("completing the last transaction") {
                    poll.timer.fire();
                    i2c_queue.try_read(&empty_msg);
                    auto txn =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);
                    txn.handle_buffer(first_response);
                    txn.client_callback();
                    i2c_queue.try_read(&empty_msg);
                    txn =
                        std::get<pipette_messages::TransactWithI2C>(empty_msg);

                    txn.handle_buffer(second_response);
                    txn.client_callback();
                    THEN("the upstream client callback is called") {
                        REQUIRE(client_cb_call_count == 1);
                    }
                    THEN("the poll is no longer active") {
                        REQUIRE(!poll.timer.is_running());
                        REQUIRE(poll.address == 0);
                    }
                }
            }
        }
    }
}

template <typename M>
int get_period(const M& message) {
    return message.delay_ms;
}

template <>
int get_period(const std::monostate& _) {
    static_cast<void>(_);
    return 0;
}

template <typename M>
uint16_t get_address(const M& message) {
    return message.address;
}

template <>
uint16_t get_address(const std::monostate& _) {
    static_cast<void>(_);
    return 0;
}

template <typename M>
uint32_t get_poll_id(const M& _) {
    static_cast<void>(_);
    return 0;
}

template <>
uint32_t get_poll_id(
    const pipette_messages::ConfigureSingleRegisterContinuousPolling& message) {
    return message.poll_id;
}

template <>
uint32_t get_poll_id(
    const pipette_messages::ConfigureMultiRegisterContinuousPolling& message) {
    return message.poll_id;
}

SCENARIO("test the ongoing i2c polling") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> i2c_queue{};

    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    auto poll_handler = i2c_poller_task::I2CPollingMessageHandler<
        test_mocks::MockMessageQueue, test_mocks::MockTimer>{writer};

    GIVEN("an address with no ongoing polling") {
        WHEN("starting a new poll") {
            auto which_msg = GENERATE(
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x1234,
                        .delay_ms = 100,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 11,
                        .address = 0x1235,
                        .delay_ms = 101,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer = [](const auto& _1, const auto& _2) {
                            static_cast<void>(_1);
                            static_cast<void>(_2);
                        }}});
            poll_handler.handle_message(which_msg);
            THEN("the first poll slot should be used") {
                REQUIRE(poll_handler.continuous_polls.polls[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   which_msg));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.get_period() ==
                    static_cast<uint32_t>(std::visit(
                        [](auto& msg) { return get_period(msg); }, which_msg)));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.is_running());
                REQUIRE(poll_handler.continuous_polls.polls[0].poll_id ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   which_msg));
            }
            THEN("no poll slot but the first should be used") {
                auto poll_iter = poll_handler.continuous_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != poll_handler.continuous_polls.polls.end();
                     poll_iter++) {
                    REQUIRE(poll_iter->address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->poll_id == 0);
                }
            }
        }
    }

    GIVEN("an address with a poll active") {
        auto original_message = GENERATE(
            i2c_poller::TaskMessage{
                std::in_place_type<
                    pipette_messages::ConfigureSingleRegisterContinuousPolling>,
                pipette_messages::ConfigureSingleRegisterContinuousPolling{
                    .poll_id = 10,
                    .address = 0x1234,
                    .delay_ms = 100,
                    .buffer = std::array<uint8_t, 5>{},
                    .handle_buffer =
                        [](const auto& _) { static_cast<void>(_); }}},
            i2c_poller::TaskMessage{
                std::in_place_type<
                    pipette_messages::ConfigureMultiRegisterContinuousPolling>,
                pipette_messages::ConfigureMultiRegisterContinuousPolling{
                    .poll_id = 10,
                    .address = 0x1234,
                    .delay_ms = 101,
                    .register_1_buffer = std::array<uint8_t, 5>{},
                    .register_2_buffer = std::array<uint8_t, 5>{},
                    .handle_buffer = [](const auto& _1, const auto& _2) {
                        static_cast<void>(_1);
                        static_cast<void>(_2);
                    }}});
        poll_handler.handle_message(original_message);
        CHECK(poll_handler.continuous_polls.polls[0].address ==
              std::visit([](auto& msg) { return get_address(msg); },
                         original_message));
        CHECK(
            poll_handler.continuous_polls.polls[0].timer.get_period() ==
            static_cast<uint32_t>(std::visit(
                [](auto& msg) { return get_period(msg); }, original_message)));
        CHECK(poll_handler.continuous_polls.polls[0].timer.is_running());
        CHECK(poll_handler.continuous_polls.polls[0].poll_id ==
              std::visit([](auto& msg) { return get_poll_id(msg); },
                         original_message));

        WHEN("sending a message to the same poll id and address") {
            auto new_message = GENERATE(
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x1234,
                        .delay_ms = 40,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x1234,
                        .delay_ms = 99,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer = [](const auto& _1, const auto& _2) {
                            static_cast<void>(_1);
                            static_cast<void>(_2);
                        }}});
            poll_handler.handle_message(new_message);
            THEN("the new message should overwrite the old one") {
                REQUIRE(poll_handler.continuous_polls.polls[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   new_message));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.get_period() ==
                    static_cast<uint32_t>(
                        std::visit([](auto& msg) { return get_period(msg); },
                                   new_message)));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.is_running());
                REQUIRE(poll_handler.continuous_polls.polls[0].poll_id ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   new_message));
            }
            THEN("no poll slot but the first should be used") {
                auto poll_iter = poll_handler.continuous_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != poll_handler.continuous_polls.polls.end();
                     poll_iter++) {
                    REQUIRE(poll_iter->address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->poll_id == 0);
                }
            }
        }
        WHEN(
            "sending a message to the same poll id and address with period 0") {
            auto new_message = GENERATE(
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x1234,
                        .delay_ms = 0,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x1234,
                        .delay_ms = 0,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer = [](const auto& _1, const auto& _2) {
                            static_cast<void>(_1);
                            static_cast<void>(_2);
                        }}});
            poll_handler.handle_message(new_message);
            THEN("the polling should stop and the slot should be freed") {
                REQUIRE(poll_handler.continuous_polls.polls[0].address == 0);
                REQUIRE(poll_handler.continuous_polls.polls[0].poll_id == 0);
                REQUIRE(
                    !poll_handler.continuous_polls.polls[0].timer.is_running());
            }
        }
        WHEN("sending a message to a different poll id or address or both") {
            auto new_message = GENERATE(
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x7777,
                        .delay_ms = 40,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 10,
                        .address = 0x7777,
                        .delay_ms = 99,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _1, const auto& _2) {
                                static_cast<void>(_1);
                                static_cast<void>(_2);
                            }}},

                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 11,
                        .address = 0x1234,
                        .delay_ms = 40,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 11,
                        .address = 0x1234,
                        .delay_ms = 99,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _1, const auto& _2) {
                                static_cast<void>(_1);
                                static_cast<void>(_2);
                            }}},

                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    pipette_messages::ConfigureSingleRegisterContinuousPolling{
                        .poll_id = 11,
                        .address = 0x7777,
                        .delay_ms = 40,
                        .buffer = std::array<uint8_t, 5>{},
                        .handle_buffer =
                            [](const auto& _) { static_cast<void>(_); }}},
                i2c_poller::TaskMessage{
                    std::in_place_type<
                        pipette_messages::
                            ConfigureMultiRegisterContinuousPolling>,
                    pipette_messages::ConfigureMultiRegisterContinuousPolling{
                        .poll_id = 11,
                        .address = 0x7777,
                        .delay_ms = 99,
                        .register_1_buffer = std::array<uint8_t, 5>{},
                        .register_2_buffer = std::array<uint8_t, 5>{},
                        .handle_buffer = [](const auto& _1, const auto& _2) {
                            static_cast<void>(_1);
                            static_cast<void>(_2);
                        }}});
            poll_handler.handle_message(new_message);
            THEN("the old message should still be in place") {
                REQUIRE(poll_handler.continuous_polls.polls[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   original_message));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.get_period() ==
                    static_cast<uint32_t>(
                        std::visit([](auto& msg) { return get_period(msg); },
                                   original_message)));
                REQUIRE(
                    poll_handler.continuous_polls.polls[0].timer.is_running());
                REQUIRE(poll_handler.continuous_polls.polls[0].poll_id ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   original_message));
            }
            THEN("the new message should be in a new slot") {
                REQUIRE(poll_handler.continuous_polls.polls[1].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   new_message));
                REQUIRE(
                    poll_handler.continuous_polls.polls[1].timer.get_period() ==
                    static_cast<uint32_t>(
                        std::visit([](auto& msg) { return get_period(msg); },
                                   new_message)));
                REQUIRE(
                    poll_handler.continuous_polls.polls[1].timer.is_running());
                REQUIRE(poll_handler.continuous_polls.polls[1].poll_id ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   new_message));
            }
            THEN("no poll slot but the first two should be used") {
                auto poll_iter = poll_handler.continuous_polls.polls.begin();
                poll_iter++;
                poll_iter++;
                for (; poll_iter != poll_handler.continuous_polls.polls.end();
                     poll_iter++) {
                    REQUIRE(poll_iter->address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->poll_id == 0);
                }
            }
        }
    }

    GIVEN("a configured single-register poll") {
        uint32_t buffer_cb_call_count = 0;
        std::array<uint8_t, 5> last_buffer{};
        std::array<uint8_t, 5> response_buffer{0xff, 0xfe, 0xfd, 0xfc, 0xfb};
        pipette_messages::ConfigureSingleRegisterContinuousPolling poll_msg{
            .poll_id = 10,
            .address = 0x1234,
            .delay_ms = 100,
            .buffer = std::array<uint8_t, 5>{0x1, 0x2, 0x3, 0x4, 0x5},
            .handle_buffer = [&buffer_cb_call_count, &last_buffer](
                                 const std::array<uint8_t, 5>& buffer) {
                buffer_cb_call_count++;
                last_buffer = buffer;
            }};
        auto tm = i2c_poller::TaskMessage{poll_msg};
        poll_handler.handle_message(tm);
        WHEN("the poll timer fires") {
            poll_handler.continuous_polls.polls[0].timer.fire();
            THEN("a transaction should be enqueued") {
                i2c_writer::TaskMessage empty_msg{};
                REQUIRE(i2c_queue.get_size() == 1);
                i2c_queue.try_read(&empty_msg);
                auto transaction =
                    std::get<pipette_messages::TransactWithI2C>(empty_msg);
                REQUIRE(transaction.address == poll_msg.address);
                REQUIRE(transaction.buffer == poll_msg.buffer);
                REQUIRE(buffer_cb_call_count == 0);
                AND_WHEN("calling that transaction's callback") {
                    transaction.handle_buffer(response_buffer);
                    THEN("the upstream callback should fire") {
                        REQUIRE(buffer_cb_call_count == 1);
                        REQUIRE(last_buffer == response_buffer);
                    }
                }
                AND_WHEN("the timer fires again") {
                    poll_handler.continuous_polls.polls[0].timer.fire();
                    THEN(
                        "another transaction should be enqueued with the same "
                        "stimulus") {
                        i2c_writer::TaskMessage empty_msg{};
                        REQUIRE(i2c_queue.get_size() == 1);
                        i2c_queue.try_read(&empty_msg);
                        auto transaction =
                            std::get<pipette_messages::TransactWithI2C>(
                                empty_msg);
                        REQUIRE(transaction.address == poll_msg.address);
                        REQUIRE(transaction.buffer == poll_msg.buffer);
                    }
                }
            }
        }
    }

    GIVEN("a configured multi-register poll") {
        uint32_t buffer_cb_call_count = 0;
        std::array<uint8_t, 5> last_buffer_1{};
        std::array<uint8_t, 5> last_buffer_2{};
        std::array<uint8_t, 5> response_buffer_1{0xff, 0xfe, 0xfd, 0xfc, 0xfb};
        std::array<uint8_t, 5> response_buffer_2{0xf, 0xe, 0xd, 0xc, 0xb};
        pipette_messages::ConfigureMultiRegisterContinuousPolling poll_msg{
            .poll_id = 10,
            .address = 0x1234,
            .delay_ms = 100,
            .register_1_buffer =
                std::array<uint8_t, 5>{0x1, 0x2, 0x3, 0x4, 0x5},
            .register_2_buffer =
                std::array<uint8_t, 5>{0x6, 0x7, 0x8, 0x9, 0xa},
            .handle_buffer = [&buffer_cb_call_count, &last_buffer_1,
                              &last_buffer_2](
                                 const std::array<uint8_t, 5>& buffer_1,
                                 const std::array<uint8_t, 5>& buffer_2) {
                buffer_cb_call_count++;
                last_buffer_1 = buffer_1;
                last_buffer_2 = buffer_2;
            }};
        auto tm = i2c_poller::TaskMessage{poll_msg};
        poll_handler.handle_message(tm);
        WHEN("the poll timer fires") {
            poll_handler.continuous_polls.polls[0].timer.fire();
            THEN("a transaction should be enqueued") {
                i2c_writer::TaskMessage empty_msg{};
                REQUIRE(i2c_queue.get_size() == 1);
                i2c_queue.try_read(&empty_msg);
                auto transaction =
                    std::get<pipette_messages::TransactWithI2C>(empty_msg);
                REQUIRE(transaction.address == poll_msg.address);
                REQUIRE(transaction.buffer == poll_msg.register_1_buffer);
                REQUIRE(buffer_cb_call_count == 0);
                AND_WHEN("calling that transaction's callback") {
                    transaction.handle_buffer(response_buffer_1);
                    THEN("the upstream callback should not fire yet") {
                        REQUIRE(buffer_cb_call_count == 0);
                    }
                    THEN("another transaction should be enqueued") {
                        REQUIRE(i2c_queue.get_size() == 1);
                        i2c_queue.try_read(&empty_msg);
                        auto transaction =
                            std::get<pipette_messages::TransactWithI2C>(
                                empty_msg);
                        REQUIRE(transaction.address == poll_msg.address);
                        REQUIRE(transaction.buffer ==
                                poll_msg.register_2_buffer);
                        REQUIRE(buffer_cb_call_count == 0);
                        AND_WHEN("calling that transaction's callback") {
                            transaction.handle_buffer(response_buffer_2);
                            THEN("the upstream callback should be called") {
                                REQUIRE(buffer_cb_call_count == 1);
                                REQUIRE(last_buffer_1 == response_buffer_1);
                                REQUIRE(last_buffer_2 == response_buffer_2);
                                REQUIRE(i2c_queue.get_size() == 0);
                                AND_WHEN("the timer fires again") {
                                    poll_handler.continuous_polls.polls[0]
                                        .timer.fire();
                                    THEN(
                                        "another transaction should be "
                                        "enqueued with the same stimulus") {
                                        i2c_writer::TaskMessage empty_msg{};
                                        REQUIRE(i2c_queue.get_size() == 1);
                                        i2c_queue.try_read(&empty_msg);
                                        auto transaction = std::get<
                                            pipette_messages::TransactWithI2C>(
                                            empty_msg);
                                        REQUIRE(transaction.address ==
                                                poll_msg.address);
                                        REQUIRE(transaction.buffer ==
                                                poll_msg.register_1_buffer);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
