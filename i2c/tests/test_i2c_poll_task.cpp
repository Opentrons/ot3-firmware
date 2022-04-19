#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_timer.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

#define u8(X) static_cast<uint8_t>(X)

template <typename Message>
auto get_message(test_mocks::MockMessageQueue<i2c::poller::TaskMessage>& q)
    -> Message {
    i2c::poller::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

template <typename Message>
auto get_message(test_mocks::MockMessageQueue<i2c::writer::TaskMessage>& q)
    -> Message {
    i2c::writer::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

SCENARIO("test the limited-count i2c poller") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};

    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poll_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};

    auto poll_handler =
        i2c::tasks::I2CPollerMessageHandler<test_mocks::MockMessageQueue,
                                            test_mocks::MockTimer,
                                            decltype(poll_queue)>{writer,
                                                                  poll_queue};

    GIVEN("single register limited poll command") {
        uint16_t addr = 0x1234;
        int poll_count = 10;
        int delay = 100;
        i2c::messages::Transaction original_txn = {
            .address = addr,
            .bytes_to_read = 4,
            .bytes_to_write = 3,
            .write_buffer =
                std::array{u8(0x2), u8(0x3), u8(0x4), u8(0x5), u8(0x6)},
        };
        i2c::messages::SingleRegisterPollRead msg{
            .polling = poll_count,
            .delay_ms = delay,
            .first = original_txn,
            .id = {.token = 12314, .is_completed_poll = false},
            .response_writer = i2c::messages::ResponseWriter(response_queue)};
        auto tm = i2c::poller::TaskMessage{msg};
        WHEN("executing the command") {
            poll_handler.handle_message(tm);
            auto& limited_polls = poll_handler.limited_polls;
            auto& poll = limited_polls.polls[0];

            THEN("nothing is immediately enqueued") {
                REQUIRE(!i2c_queue.has_message());
            }
            THEN("a poll object is provisioned with a timer") {
                REQUIRE(poll.transactions[0] == original_txn);
                REQUIRE(poll.transactions[1].address == 0);
                REQUIRE(poll.timer.get_period() ==
                        static_cast<uint32_t>(delay));
                REQUIRE(poll.timer.is_running());
            }
            THEN("only one poll object is provisioned") {
                auto poll_iter = limited_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != limited_polls.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->transactions[0].address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                }
            }
            AND_WHEN("firing the timer once") {
                poll.timer.fire();
                THEN("a correct transaction is sent to the i2c task") {
                    REQUIRE(i2c_queue.get_size() == 1);
                    auto transaction =
                        get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(transaction.transaction == original_txn);
                    AND_WHEN("that transaction is responded to") {
                        std::array<uint8_t, 5> response_buffer = {0xf, 0xe, 0xd,
                                                                  0xc, 0xb};
                        i2c::messages::TransactionResponse response{
                            .id = transaction.id,
                            .bytes_read = 4,
                            .read_buffer = response_buffer};
                        static_cast<void>(transaction.response_writer.write(response));
                        auto first_resp =
                            get_message<i2c::messages::TransactionResponse>(
                                poll_queue);
                        tm = first_resp;
                        poll_handler.handle_message(tm);
                        THEN("then another response is sent upstream") {
                            auto resp =
                                test_mocks::get_response(response_queue);
                            REQUIRE(resp.read_buffer == response.read_buffer);
                            REQUIRE(resp.bytes_read == response.bytes_read);
                            REQUIRE(resp.id.token == transaction.id.token);
                        }
                    }
                }
            }
            AND_WHEN("firing the timer poll_count-1 times") {
                std::array<uint8_t, 5> response_buffer{0xaa, 0xbb, 0xcc, 0xdd,
                                                       0xee};
                i2c::messages::TransactionResponse response{
                    .id = msg.id,
                    .bytes_read = msg.first.bytes_to_read,
                    .read_buffer = response_buffer,
                };

                for (int count = 0; count < (poll_count - 1); count++) {
                    poll.timer.fire();
                    auto msg = get_message<i2c::messages::Transact>(i2c_queue);
                    CHECK(msg.transaction == original_txn);
                    CHECK(msg.id.token == 12314);
                    CHECK(msg.id.is_completed_poll == false);
                    static_cast<void>(msg.response_writer.write(response));
                    auto resp = get_message<i2c::messages::TransactionResponse>(
                        poll_queue);
                    i2c::poller::TaskMessage tm{resp};
                    poll_handler.handle_message(tm);
                }
                THEN(
                    "there are the right number of upstreamed messages and "
                    "none are complete") {
                    REQUIRE(response_queue.get_size() == (poll_count - 1));
                    for (int i = 0; i < (poll_count - 1); i++) {
                        auto msg = test_mocks::get_response(response_queue);
                        REQUIRE(msg.id.is_completed_poll == false);
                        REQUIRE(msg.id.token == 12314);
                        REQUIRE(msg.read_buffer == response_buffer);
                        REQUIRE(msg.bytes_read == original_txn.bytes_to_read);
                    }

                    AND_WHEN("firing the last poll") {
                        poll.timer.fire();
                        auto last_txn =
                            get_message<i2c::messages::Transact>(i2c_queue);
                        std::array<uint8_t, 5> response_buffer = {0xf, 0xe, 0xd,
                                                                  0xc, 0xb};
                        response = {
                            .id = last_txn.id,
                            .bytes_read = 3,
                            .read_buffer = response_buffer,
                        };
                        CHECK(last_txn.id.is_completed_poll == true);
                        static_cast<void>(last_txn.response_writer.write(response));
                        auto resp =
                            get_message<i2c::messages::TransactionResponse>(
                                poll_queue);
                        tm = resp;
                        poll_handler.handle_message(tm);
                        THEN("the upstream message is completed") {
                            auto upstream =
                                test_mocks::get_response(response_queue);
                            REQUIRE(upstream.id.token == 12314);
                            REQUIRE(upstream.id.is_completed_poll == true);
                        }
                        THEN("the poll is no longer active") {
                            REQUIRE(!poll.timer.is_running());
                            REQUIRE(poll.transactions[0].address == 0);
                        }
                    }
                }
            }
        }
    }

    GIVEN("multi register limited poll command") {
        uint16_t addr = 0x1234;
        std::array<uint8_t, 5> register_payload_1{0x2, 0x3, 0x4, 0x5, 0x6};
        std::array<uint8_t, 5> register_payload_2{0x4, 0x6, 0x8, 0xa, 0xc};
        int poll_count = 10;
        int delay = 100;
        i2c::messages::Transaction original_txn_1{
            .address = addr,
            .bytes_to_read = 3,
            .bytes_to_write = 5,
            .write_buffer = register_payload_1};
        i2c::messages::Transaction original_txn_2{
            .address = addr,
            .bytes_to_read = 3,
            .bytes_to_write = 5,
            .write_buffer = register_payload_2};
        i2c::messages::MultiRegisterPollRead msg{
            .polling = poll_count,
            .delay_ms = delay,
            .id = {.token = 12345, .is_completed_poll = false},
            .first = original_txn_1,
            .second = original_txn_2,
            .response_writer = i2c::messages::ResponseWriter(response_queue)};
        auto tm = i2c::poller::TaskMessage{msg};
        WHEN("executing the command") {
            poll_handler.handle_message(tm);
            auto& limited_polls = poll_handler.limited_polls;
            auto& poll = limited_polls.polls[0];

            THEN("nothing is immediately enqueued") {
                REQUIRE(!i2c_queue.has_message());
            }
            THEN("a poll object is provisioned with a timer") {
                REQUIRE(poll.transactions[0].address == addr);
                REQUIRE(poll.timer.get_period() ==
                        static_cast<uint32_t>(delay));
                REQUIRE(poll.timer.is_running());
            }
            THEN("only one poll object is provisioned") {
                auto poll_iter = limited_polls.polls.begin();
                poll_iter++;
                for (; poll_iter != limited_polls.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->transactions[0].address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                }
            }
            AND_WHEN("firing the timer once") {
                poll.timer.fire();
                THEN(
                    "a correct first-reg transaction is sent to the i2c task") {
                    REQUIRE(i2c_queue.get_size() == 1);
                    auto msg = get_message<i2c::messages::Transact>(i2c_queue);
                    REQUIRE(msg.transaction == original_txn_1);
                    REQUIRE(msg.id.token == 12345);
                    REQUIRE(msg.id.is_completed_poll == false);
                    REQUIRE(msg.id.transaction_index == 0);
                    AND_WHEN("that transaction is responded to") {
                        std::array<uint8_t, 5> response_buf_1 = {0xf, 0xe, 0xd,
                                                                 0xc, 0xb};
                        i2c::messages::TransactionResponse response_1{
                            .id = msg.id,
                            .bytes_read = 4,
                            .read_buffer = response_buf_1};
                        static_cast<void>(msg.response_writer.write(response_1));
                        auto next =
                            get_message<i2c::messages::TransactionResponse>(
                                poll_queue);
                        tm = i2c::poller::TaskMessage(next);
                        poll_handler.handle_message(tm);
                        auto resp = test_mocks::get_response(response_queue);
                        THEN("an upstream response is sent") {
                            REQUIRE(resp.read_buffer == response_buf_1);
                            REQUIRE(resp.bytes_read == response_1.bytes_read);
                            REQUIRE(resp.id.transaction_index == 0);
                            REQUIRE(resp.id.token == 12345);
                            REQUIRE(resp.id.is_completed_poll == false);
                        }
                        THEN(
                            "a correct second-reg transaction is sent to the "
                            "i2c task") {
                            msg =
                                get_message<i2c::messages::Transact>(i2c_queue);
                            REQUIRE(msg.transaction.bytes_to_write ==
                                    original_txn_2.bytes_to_write);
                            REQUIRE(msg.transaction.bytes_to_read ==
                                    original_txn_2.bytes_to_read);
                            REQUIRE(msg.transaction.address ==
                                    original_txn_2.address);
                            REQUIRE(msg.transaction.write_buffer ==
                                    original_txn_2.write_buffer);
                            REQUIRE(msg.transaction == original_txn_2);
                            REQUIRE(msg.id.token == 12345);
                            REQUIRE(msg.id.transaction_index == 1);
                            REQUIRE(msg.id.is_completed_poll == false);
                            AND_WHEN("that second transaction is handled") {
                                std::array<uint8_t, 5> response_buf_2 = {
                                    0xff, 0xee, 0xdd, 0xcc, 0xbb};
                                i2c::messages::TransactionResponse response_2{
                                    .id = msg.id,
                                    .bytes_read = msg.transaction.bytes_to_read,
                                    .read_buffer = response_buf_2};
                                static_cast<void>(msg.response_writer.write(response_2));
                                next = get_message<
                                    i2c::messages::TransactionResponse>(
                                    poll_queue);
                                tm = i2c::poller::TaskMessage(next);
                                poll_handler.handle_message(tm);
                                THEN(
                                    "a response is sent with the right index") {
                                    auto resp = test_mocks::get_response(
                                        response_queue);
                                    REQUIRE(resp.id.token == 12345);
                                    REQUIRE(resp.id.is_completed_poll == false);
                                    REQUIRE(resp.id.transaction_index == 1);
                                    REQUIRE(resp.read_buffer == response_buf_2);
                                    REQUIRE(resp.bytes_read ==
                                            msg.transaction.bytes_to_read);
                                }
                            }
                        }
                    }
                }
            }
            AND_WHEN("firing the timer enough times to exhaust the poll") {
                std::array<uint8_t, 5> first_response_buf = {0xf, 0xe, 0xd, 0xc,
                                                             0xb};
                std::array<uint8_t, 5> second_response_buf = {0xff, 0xee, 0xdd,
                                                              0xcc, 0xbb};
                for (int count = 0; count < (poll_count - 1); count++) {
                    poll.timer.fire();
                    auto txn = get_message<i2c::messages::Transact>(i2c_queue);
                    i2c::messages::TransactionResponse first_response{
                        .id = txn.id,
                        .bytes_read = 5,
                        .read_buffer = first_response_buf};
                    static_cast<void>(txn.response_writer.write(first_response));
                    auto next = get_message<i2c::messages::TransactionResponse>(
                        poll_queue);
                    CHECK(next == first_response);
                    i2c::poller::TaskMessage tm(next);
                    poll_handler.handle_message(tm);

                    txn = get_message<i2c::messages::Transact>(i2c_queue);
                    i2c::messages::TransactionResponse second_response{
                        .id = txn.id,
                        .bytes_read = 5,
                        .read_buffer = second_response_buf};
                    static_cast<void>(txn.response_writer.write(second_response));
                    next = get_message<i2c::messages::TransactionResponse>(
                        poll_queue);
                    CHECK(next == second_response);
                    tm = next;
                    poll_handler.handle_message(tm);
                    CHECK(i2c_queue.get_size() == 0);
                }
                THEN(
                    "the correct number of transactions were sent to the i2c "
                    "task") {
                    REQUIRE(response_queue.get_size() ==
                            (static_cast<uint32_t>(poll_count - 1) * 2));
                    for (int count = 0; count < ((poll_count - 1) * 2);
                         count++) {
                        auto upstream = get_response(response_queue);
                        REQUIRE(upstream.id.token == 12345);
                        REQUIRE(upstream.id.is_completed_poll == false);
                        if (count % 2) {
                            REQUIRE(upstream.id.transaction_index == 1);
                            REQUIRE(upstream.read_buffer ==
                                    second_response_buf);
                        } else {
                            REQUIRE(upstream.id.transaction_index == 0);
                            REQUIRE(upstream.read_buffer == first_response_buf);
                        }
                    }
                    AND_WHEN("completing the last transaction") {
                        poll.timer.fire();
                        auto txn =
                            get_message<i2c::messages::Transact>(i2c_queue);
                        i2c::messages::TransactionResponse first_response{
                            .id = txn.id,
                            .bytes_read = 5,
                            .read_buffer = first_response_buf};
                        static_cast<void>(txn.response_writer.write(first_response));
                        auto next =
                            get_message<i2c::messages::TransactionResponse>(
                                poll_queue);
                        CHECK(next == first_response);
                        i2c::poller::TaskMessage tm(next);
                        poll_handler.handle_message(tm);
                        txn = get_message<i2c::messages::Transact>(i2c_queue);
                        i2c::messages::TransactionResponse second_response{
                            .id = txn.id,
                            .bytes_read = 5,
                            .read_buffer = second_response_buf};
                        static_cast<void>(txn.response_writer.write(second_response));
                        next = get_message<i2c::messages::TransactionResponse>(
                            poll_queue);
                        CHECK(next == second_response);
                        tm = next;
                        poll_handler.handle_message(tm);
                        CHECK(i2c_queue.get_size() == 0);
                        CHECK(response_queue.get_size() == 2);
                        THEN("two good messages are passed upstream") {
                            auto upstream =
                                test_mocks::get_response(response_queue);
                            REQUIRE(upstream.id.transaction_index == 0);
                            REQUIRE(upstream.id.is_completed_poll == false);
                            REQUIRE(upstream.id.token == 12345);
                            REQUIRE(upstream.read_buffer == first_response_buf);
                            upstream = test_mocks::get_response(response_queue);
                            REQUIRE(upstream.id.transaction_index == 1);
                            REQUIRE(upstream.id.is_completed_poll == true);
                            REQUIRE(upstream.id.token == 12345);
                            REQUIRE(upstream.read_buffer ==
                                    second_response_buf);
                        }
                        THEN("the poll is no longer active") {
                            REQUIRE(!poll.timer.is_running());
                            REQUIRE(poll.transactions[0].address == 0);
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("test the ongoing i2c polling") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};

    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&i2c_queue);

    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> poll_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};

    auto poll_handler =
        i2c::tasks::I2CPollerMessageHandler<test_mocks::MockMessageQueue,
                                            test_mocks::MockTimer,
                                            decltype(poll_queue)>{writer,
                                                                  poll_queue};

    GIVEN("a configured single-register poll") {
        i2c::messages::ConfigureSingleRegisterContinuousPolling poll_msg{
            .delay_ms = 100,
            .first = {.address = 0x1234,
                      .bytes_to_read = 3,
                      .bytes_to_write = 5,
                      .write_buffer =
                          std::array<uint8_t, 5>{0x1, 0x2, 0x3, 0x4, 0x5}},
            .id = {.token = 12345,
                   .is_completed_poll = false,
                   .transaction_index = 0},
            .response_writer = i2c::messages::ResponseWriter(response_queue)};
        auto tm = i2c::poller::TaskMessage{poll_msg};
        poll_handler.handle_message(tm);
        WHEN("the poll timer fires") {
            poll_handler.continuous_polls.polls[0].timer.fire();
            THEN("a transaction should be enqueued") {
                auto transaction =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(transaction.transaction.address ==
                        poll_msg.first.address);
                REQUIRE(transaction.transaction.write_buffer ==
                        poll_msg.first.write_buffer);
                REQUIRE(transaction.id.transaction_index == 0);
                AND_WHEN("responding to that transaction") {
                    i2c::messages::TransactionResponse response_msg{
                        .id = transaction.id,
                        .bytes_read = transaction.transaction.bytes_to_read,
                        .read_buffer = std::array<uint8_t, 5>{1, 2, 3, 4, 5}};
                    static_cast<void>(transaction.response_writer.write(response_msg));
                    auto next = get_message<i2c::messages::TransactionResponse>(
                        poll_queue);
                    REQUIRE(next == response_msg);
                    tm = next;
                    poll_handler.handle_message(tm);
                    THEN("a response should be sent upstream") {
                        auto upstream =
                            test_mocks::get_response(response_queue);
                        REQUIRE(upstream.id.token == poll_msg.id.token);
                        REQUIRE(!upstream.id.is_completed_poll);
                        REQUIRE(upstream.id.transaction_index == 0);
                        REQUIRE(upstream.read_buffer ==
                                response_msg.read_buffer);
                    }
                }
                AND_WHEN("the timer fires again") {
                    poll_handler.continuous_polls.polls[0].timer.fire();
                    THEN(
                        "another transaction should be enqueued with the same "
                        "stimulus") {
                        auto second_transaction =
                            get_message<i2c::messages::Transact>(i2c_queue);
                        REQUIRE(second_transaction.transaction ==
                                transaction.transaction);
                        REQUIRE(second_transaction.id == transaction.id);
                    }
                }
            }
        }
    }
    GIVEN("a configured multi-register poll") {
        auto first_txn = i2c::messages::Transaction{
            .address = 0x1234,
            .bytes_to_read = 3,
            .bytes_to_write = 5,
            .write_buffer = std::array<uint8_t, 5>{0x1, 0x2, 0x3, 0x4, 0x5}};
        auto second_txn = i2c::messages::Transaction{
            .address = 0x1234,
            .bytes_to_read = 3,
            .bytes_to_write = 5,
            .write_buffer = std::array<uint8_t, 5>{0x6, 0x7, 0x8, 0x9, 0xa}};
        i2c::messages::ConfigureMultiRegisterContinuousPolling poll_msg{
            .delay_ms = 100,
            .first = first_txn,
            .second = second_txn,
            .id = {.token = 12314,
                   .is_completed_poll = false,
                   .transaction_index = 0},
            .response_writer = i2c::messages::ResponseWriter(response_queue)};
        auto tm = i2c::poller::TaskMessage{poll_msg};
        poll_handler.handle_message(tm);
        WHEN("the poll timer fires") {
            poll_handler.continuous_polls.polls[0].timer.fire();
            THEN("a transaction should be enqueued") {
                auto transaction =
                    get_message<i2c::messages::Transact>(i2c_queue);
                REQUIRE(transaction.transaction == first_txn);
                AND_WHEN("responding to that transaction") {
                    std::array<uint8_t, 5> first_resp_buffer{5, 4, 3, 2, 1};
                    i2c::messages::TransactionResponse first_resp{
                        .id = transaction.id,
                        .bytes_read = 3,
                        .read_buffer = first_resp_buffer};
                    static_cast<void>(transaction.response_writer.write(first_resp));
                    auto next = get_message<i2c::messages::TransactionResponse>(
                        poll_queue);
                    tm = next;
                    poll_handler.handle_message(tm);
                    auto upstream = test_mocks::get_response(response_queue);
                    THEN(
                        "a message should go upstream with the correct index") {
                        REQUIRE(upstream.id.token == poll_msg.id.token);
                        REQUIRE(upstream.id.transaction_index == 0);
                        REQUIRE(upstream.read_buffer == first_resp_buffer);
                    }
                    THEN("another transaction should be enqueued") {
                        auto second_transaction =
                            get_message<i2c::messages::Transact>(i2c_queue);
                        REQUIRE(second_transaction.transaction == second_txn);
                        AND_WHEN("responding to the second transaction") {
                            std::array<uint8_t, 5> second_resp_buffer{10, 9, 8,
                                                                      7, 6};
                            i2c::messages::TransactionResponse second_resp{
                                .id = second_transaction.id,
                                .bytes_read = 3,
                                .read_buffer = second_resp_buffer};
                            static_cast<void>(second_transaction.response_writer.write(
                                second_resp));
                            next =
                                get_message<i2c::messages::TransactionResponse>(
                                    poll_queue);
                            REQUIRE(next == second_resp);
                            tm = second_resp;
                            poll_handler.handle_message(tm);
                            upstream = test_mocks::get_response(response_queue);
                            THEN(
                                "a message should go upstream with the correct "
                                "index") {
                                REQUIRE(upstream.id.token == poll_msg.id.token);
                                REQUIRE(upstream.id.transaction_index == 1);
                                REQUIRE(upstream.read_buffer ==
                                        second_resp_buffer);
                                AND_WHEN("the timer fires again") {
                                    poll_handler.continuous_polls.polls[0]
                                        .timer.fire();
                                    THEN(
                                        "another transaction should be "
                                        "enqueued with the first") {
                                        auto transaction = get_message<
                                            i2c::messages::Transact>(i2c_queue);
                                        REQUIRE(transaction.transaction ==
                                                first_txn);
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
