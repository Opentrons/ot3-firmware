#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "i2c/tests/mock_response_queue.hpp"

#define u8(X) static_cast<uint8_t>(X)

template <typename Message, typename Queue>
auto get_message(Queue& q) -> Message {
    i2c::poller::TaskMessage empty_msg{};
    q.try_read(&empty_msg);
    return std::get<Message>(empty_msg);
}

SCENARIO("Test the i2c poller command queue") {
    test_mocks::MockMessageQueue<i2c::poller::TaskMessage> queue{};
    auto poller = i2c::poller::Poller<test_mocks::MockMessageQueue>{};
    poller.set_queue(&queue);
    GIVEN("An i2c command queue poller to do single register limited-polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            poller.single_register_poll(ADDRESS, small, 4, 20, 15, queue);
            auto poll_msg =
                get_message<i2c::messages::SingleRegisterPollRead>(queue);
            THEN("the top data members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
                REQUIRE(poll_msg.polling == 20);
            }
            THEN("the transaction is correct") {
                REQUIRE(poll_msg.first.address == ADDRESS);
                REQUIRE(poll_msg.first.bytes_to_read == 4);
                REQUIRE(poll_msg.first.bytes_to_write == 1);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(5), u8(0), u8(0), u8(0), u8(0)});
            }
            THEN("the id is defaulted") {
                REQUIRE(poll_msg.id.token == 0);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
        WHEN("we request a poll with the data overload") {
            uint64_t data = 0xc0ffee44d34db33f;
            poller.single_register_poll(ADDRESS, data, 6, 20, 15, queue, 2015);
            auto poll_msg =
                get_message<i2c::messages::SingleRegisterPollRead>(queue);
            THEN("the top data members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
                REQUIRE(poll_msg.polling == 20);
            }
            THEN("the transaction is correct and limited") {
                REQUIRE(poll_msg.first.address == ADDRESS);
                REQUIRE(poll_msg.first.bytes_to_read == 5);
                REQUIRE(poll_msg.first.bytes_to_write == 5);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0xc0), u8(0xff), u8(0xee), u8(0x44),
                                   u8(0xd3)});
            }
            THEN("the id is passed along") {
                REQUIRE(poll_msg.id.token == 2015);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
    }

    GIVEN(
        "An i2c command queue poller to do single register continuous-polls") {
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            poller.continuous_single_register_poll(ADDRESS, small, 4, 15, queue,
                                                   15);
            auto poll_msg = get_message<
                i2c::messages::ConfigureSingleRegisterContinuousPolling>(queue);
            THEN("the top data members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
            }
            THEN("the transaction is correct") {
                REQUIRE(poll_msg.first.address == ADDRESS);
                REQUIRE(poll_msg.first.bytes_to_read == 4);
                REQUIRE(poll_msg.first.bytes_to_write == 1);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(5), u8(0), u8(0), u8(0), u8(0)});
            }
            THEN("the id is passed through") {
                REQUIRE(poll_msg.id.token == 15);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
        WHEN("we request a poll with the data overload") {
            uint64_t data = 0xc0ffee44d34db33f;
            poller.continuous_single_register_poll(ADDRESS, data, 6, 15, queue,
                                                   2015);
            auto poll_msg = get_message<
                i2c::messages::ConfigureSingleRegisterContinuousPolling>(queue);
            THEN("the top data members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
            }
            THEN("the transaction is correct and limited") {
                REQUIRE(poll_msg.first.address == ADDRESS);
                REQUIRE(poll_msg.first.bytes_to_read == 5);
                REQUIRE(poll_msg.first.bytes_to_write == 5);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0xc0), u8(0xff), u8(0xee), u8(0x44),
                                   u8(0xd3)});
            }
            THEN("the id is passed along") {
                REQUIRE(poll_msg.id.token == 2015);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
    }

    GIVEN("An i2c command queue poller to do multi register limited-polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small_1{u8(0x05)}, small_2{u8(0x06)};
            poller.multi_register_poll(ADDRESS, small_1, 2, small_2, 3, 20, 15,
                                       queue);
            auto poll_msg =
                get_message<i2c::messages::MultiRegisterPollRead>(queue);
            THEN("the top level members are correct") {
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }
            THEN("the transactions are correct") {
                REQUIRE(poll_msg.first.bytes_to_write == 1);
                REQUIRE(poll_msg.first.bytes_to_read == 2);
                REQUIRE(poll_msg.first.write_buffer[0] == 5);
                REQUIRE(poll_msg.second.bytes_to_write == 1);
                REQUIRE(poll_msg.second.bytes_to_read == 3);
                REQUIRE(poll_msg.second.write_buffer[0] == 6);
            }
            THEN("the id is defaulted") {
                REQUIRE(poll_msg.id.token == 0);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data_1 = 22, data_2 = 512;
            poller.multi_register_poll(ADDRESS, data_1, 6, data_2, 2, 20, 15,
                                       queue, 15);
            auto poll_msg =
                get_message<i2c::messages::MultiRegisterPollRead>(queue);
            THEN("the top level members are correct") {
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }

            THEN("the transactions are correct") {
                REQUIRE(poll_msg.first.bytes_to_write == 4);
                REQUIRE(poll_msg.first.bytes_to_read == 5);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0), u8(0), u8(0), u8(22), u8(0)});
                REQUIRE(poll_msg.second.bytes_to_write == 4);
                REQUIRE(poll_msg.second.bytes_to_read == 2);
                REQUIRE(poll_msg.second.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0), u8(0), u8(2), u8(0), u8(0)});
            }
            THEN("the id is passed through") {
                REQUIRE(poll_msg.id.token == 15);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
    }

    GIVEN("An i2c command queue poller to do multi register continuous polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small_1{u8(0x05)}, small_2{u8(0x06)};
            poller.continuous_multi_register_poll(ADDRESS, small_1, 2, small_2,
                                                  3, 15, queue, 200);
            auto poll_msg = get_message<
                i2c::messages::ConfigureMultiRegisterContinuousPolling>(queue);
            THEN("the top level members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
            }
            THEN("the transactions are correct") {
                REQUIRE(poll_msg.first.bytes_to_write == 1);
                REQUIRE(poll_msg.first.bytes_to_read == 2);
                REQUIRE(poll_msg.first.write_buffer[0] == 5);
                REQUIRE(poll_msg.second.bytes_to_write == 1);
                REQUIRE(poll_msg.second.bytes_to_read == 3);
                REQUIRE(poll_msg.second.write_buffer[0] == 6);
            }
            THEN("the id is defaulted") {
                REQUIRE(poll_msg.id.token == 200);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data_1 = 22, data_2 = 512;
            poller.continuous_multi_register_poll(ADDRESS, data_1, 6, data_2, 2,
                                                  15, queue, 15);
            auto poll_msg = get_message<
                i2c::messages::ConfigureMultiRegisterContinuousPolling>(queue);
            THEN("the transactions are correct") {
                REQUIRE(poll_msg.first.bytes_to_write == 4);
                REQUIRE(poll_msg.first.bytes_to_read == 5);
                REQUIRE(poll_msg.first.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0), u8(0), u8(0), u8(22), u8(0)});
                REQUIRE(poll_msg.second.bytes_to_write == 4);
                REQUIRE(poll_msg.second.bytes_to_read == 2);
                REQUIRE(poll_msg.second.write_buffer ==
                        i2c::messages::MaxMessageBuffer{u8(0), u8(0), u8(2), u8(0), u8(0)});
            }
            THEN("the top level members are correct") {
                REQUIRE(poll_msg.delay_ms == 15);
            }
            THEN("the id is passed through") {
                REQUIRE(poll_msg.id.token == 15);
                REQUIRE(poll_msg.id.is_completed_poll == false);
            }
            THEN("the response writer is present") {
                CHECK(poll_msg.response_writer.queue_ref != nullptr);
                CHECK(poll_msg.response_writer.writer != nullptr);
                AND_WHEN("sending a response to the responder") {
                    auto resp = i2c::messages::TransactionResponse{
                        .id = {.token = 1231, .is_completed_poll = false},
                        .bytes_read = 2,
                        .read_buffer =
                            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(0), u8(0), u8(0)},
                    };
                    static_cast<void>(poll_msg.response_writer.write(resp));
                    THEN("the response is passed along correctly") {
                        auto resp_msg =
                            get_message<i2c::messages::TransactionResponse>(
                                queue);
                        REQUIRE(resp_msg == resp);
                    }
                }
            }
        }
    }
}
