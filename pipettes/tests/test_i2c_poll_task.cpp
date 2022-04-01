#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_timer.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/messages.hpp"
#include "pipettes/core/tasks/i2c_poll_task.hpp"

SCENARIO("test the i2c poller") {
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
