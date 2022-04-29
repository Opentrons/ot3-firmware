#include <array>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

#define u8(X) static_cast<uint8_t>(X)

template <typename Queue>
auto get_message(Queue& queue) -> i2c::messages::Transact {
    CHECK(queue.get_size() == 1);
    i2c::writer::TaskMessage task_msg;
    queue.try_read(&task_msg);
    return std::get<i2c::messages::Transact>(task_msg);
}

SCENARIO("Test the i2c command queue writer") {
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> queue{};
    auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&queue);
    i2c::writer::TaskMessage empty_msg{};
    GIVEN("the desire to write stuff") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;

        WHEN("we write a uint8_t") {
            writer.write(ADDRESS, static_cast<uint8_t>(0x05));
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 1);
                REQUIRE(msg.transaction.bytes_to_read == 0);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
        WHEN("we write a uint32_t") {
            writer.write(ADDRESS, static_cast<uint32_t>(0xd34db33f));
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 4);
                REQUIRE(msg.transaction.bytes_to_read == 0);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
        WHEN("we write a two byte buffer") {
            writer.write(ADDRESS, std::array{0x1, 0x02});
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare =
                    i2c::messages::MaxMessageBuffer{u8(0x1), u8(0x2)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 2);
                REQUIRE(msg.transaction.bytes_to_read == 0);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
        WHEN("we write a full buffer") {
            writer.write(ADDRESS, std::array{u8(0x01), u8(0x02), u8(0x03),
                                             u8(0x04), u8(0x05)});
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(0x5)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 5);
                REQUIRE(msg.transaction.bytes_to_read == 0);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
        WHEN("we try and write a byte buffer that is too big") {
            auto too_big =
                std::array<uint8_t, i2c::messages::MAX_BUFFER_SIZE + 2>{};
            too_big.fill(0x12);
            writer.write(ADDRESS, too_big);

            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{};
                compare.fill(0x12);
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write ==
                        i2c::messages::MAX_BUFFER_SIZE);
                REQUIRE(msg.transaction.bytes_to_read == 0);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
        WHEN("we write a register") {
            writer.write(ADDRESS, 0x23, 0xd34db33f);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x23), u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 5);
                REQUIRE(msg.transaction.bytes_to_read == 0);
            }
            THEN("the response is empty") {
                REQUIRE(msg.response_writer.queue_ref == nullptr);
                REQUIRE(msg.response_writer.writer == nullptr);
            }
        }
    }
    GIVEN("a desire to read stuff") {
        auto response_queue = test_mocks::MockI2CResponseQueue{};
        uint16_t ADDRESS = 0x1241;
        WHEN("we command a read of <= max size") {
            writer.read(ADDRESS, 5, response_queue);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                REQUIRE(msg.transaction.bytes_to_read == 5);
                REQUIRE(msg.transaction.bytes_to_write == 0);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is correct (0)") { REQUIRE(msg.id.token == 0); }
            THEN("the response is present") {
                REQUIRE(msg.response_writer.queue_ref != nullptr);
                REQUIRE(msg.response_writer.writer != nullptr);
                AND_WHEN("we try and write a response") {
                    auto check_buf = i2c::messages::MaxMessageBuffer{
                        u8(1), u8(2), u8(3), u8(4), u8(5)};
                    static_cast<void>(msg.response_writer.write(
                        i2c::messages::TransactionResponse{
                            .id = {.token = 25, .is_completed_poll = false},
                            .bytes_read = 5,
                            .read_buffer = check_buf}));
                    auto resp = test_mocks::get_response(response_queue);
                    THEN("the response is correct") {
                        REQUIRE(resp.id.token == 25);
                        REQUIRE(resp.bytes_read == 5);
                        REQUIRE(resp.read_buffer == check_buf);
                    }
                }
            }
        }
        WHEN("we command a read of > max size") {
            writer.read(0x1234, i2c::messages::MAX_BUFFER_SIZE + 5,
                        response_queue);
            auto msg = get_message(queue);
            THEN("the transaction is limited to max size") {
                REQUIRE(msg.transaction.bytes_to_read ==
                        i2c::messages::MAX_BUFFER_SIZE);
            }
        }
        WHEN("we specify a transaction token") {
            writer.read(0x1234, 6, response_queue, 52);
            auto msg = get_message(queue);
            THEN("the token is used") { REQUIRE(msg.id.token == 52); }
        }
        WHEN("we command a read from a specified register") {
            writer.read(0x4321, 3, 5, response_queue);
            auto msg = get_message(queue);
            THEN("the transaction contains a write portion for the address") {
                REQUIRE(msg.transaction.bytes_to_write == 1);
                REQUIRE(msg.transaction.write_buffer[0] == 3);
            }
            THEN("the device address is set") {
                REQUIRE(msg.transaction.address == 0x4321);
            }
            THEN("the read count is set") {
                REQUIRE(msg.transaction.bytes_to_read == 5);
            }
        }
    }

    GIVEN("a desire to transact") {
        constexpr uint16_t ADDRESS = 0x1;
        auto response_queue = test_mocks::MockI2CResponseQueue{};
        // test add to int and add to buffer
        WHEN("we transact a uint8_t") {
            writer.transact(ADDRESS, static_cast<uint8_t>(0x05), 4,
                            response_queue);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 1);
                REQUIRE(msg.transaction.bytes_to_read == 4);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is defaulted") {
                REQUIRE(msg.id.token == 0);
                REQUIRE(msg.id.is_completed_poll == false);
            }
            THEN("the response is present") {
                REQUIRE(msg.response_writer.queue_ref != nullptr);
                REQUIRE(msg.response_writer.writer != nullptr);
                AND_WHEN("we write a response") {
                    auto response_buf = i2c::messages::MaxMessageBuffer{
                        u8(1), u8(2), u8(0), u8(4), u8(10)};
                    static_cast<void>(msg.response_writer.write(
                        i2c::messages::TransactionResponse{
                            .id = {.token = 10, .is_completed_poll = false},
                            .bytes_read = 5,
                            .read_buffer = response_buf}));
                    auto resp = test_mocks::get_response(response_queue);
                    THEN("the response is correct") {
                        REQUIRE(resp.id.token == 10);
                        REQUIRE(resp.id.is_completed_poll == false);
                        REQUIRE(resp.bytes_read == 5);
                        REQUIRE(resp.read_buffer == response_buf);
                    }
                }
            }
        }
        WHEN("we write a uint32_t and id") {
            writer.transact(ADDRESS, static_cast<uint32_t>(0xd34db33f), 3,
                            response_queue, 25);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f), u8(0)};

                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 4);
                REQUIRE(msg.transaction.bytes_to_read == 3);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is passed through") {
                REQUIRE(msg.id.token == 25);
                REQUIRE(msg.id.is_completed_poll == false);
            }
        }
        WHEN("we write a two byte buffer") {
            writer.transact(ADDRESS, std::array{0x1, 0x02}, 1, response_queue);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x1), u8(0x2), u8(0), u8(0), u8(0)};
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 2);
                REQUIRE(msg.transaction.bytes_to_read == 1);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is defaulted") {
                REQUIRE(msg.id.token == 0);
                REQUIRE(msg.id.is_completed_poll == false);
            }
            THEN("the response is present") {
                REQUIRE(msg.response_writer.queue_ref != nullptr);
                REQUIRE(msg.response_writer.writer != nullptr);
                AND_WHEN("we write a response") {
                    auto response_buf = i2c::messages::MaxMessageBuffer{
                        u8(1), u8(2), u8(0), u8(4), u8(10)};
                    static_cast<void>(msg.response_writer.write(
                        i2c::messages::TransactionResponse{
                            .id = {.token = 10, .is_completed_poll = false},
                            .bytes_read = 5,
                            .read_buffer = response_buf}));
                    auto resp = test_mocks::get_response(response_queue);
                    THEN("the response is correct") {
                        REQUIRE(resp.id.token == 10);
                        REQUIRE(resp.id.is_completed_poll == false);
                        REQUIRE(resp.bytes_read == 5);
                        REQUIRE(resp.read_buffer == response_buf);
                    }
                }
            }
        }
        WHEN("we write a five byte buffer and specify an id") {
            writer.transact(
                ADDRESS,
                std::array{u8(0x01), u8(0x02), u8(0x03), u8(0x04), u8(0x05)}, 5,
                response_queue, 651);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(5)};

                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 5);
                REQUIRE(msg.transaction.bytes_to_read == 5);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is passed through") {
                REQUIRE(msg.id.token == 651);
                REQUIRE(msg.id.is_completed_poll == false);
            }
        }
        WHEN("we try and write and read a buffer that is too big") {
            auto too_big =
                std::array<uint8_t, i2c::messages::MAX_BUFFER_SIZE + 2>{};
            too_big.fill(0x33);
            writer.transact(ADDRESS, too_big, too_big.size(), response_queue);
            auto msg = get_message(queue);
            THEN("the transaction size is limited") {
                auto compare = i2c::messages::MaxMessageBuffer{};
                compare.fill(0x33);
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write ==
                        i2c::messages::MAX_BUFFER_SIZE);
                REQUIRE(msg.transaction.bytes_to_read ==
                        i2c::messages::MAX_BUFFER_SIZE);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
        }
    }
    GIVEN("a desire to transact as a poller would") {
        constexpr uint16_t ADDRESS = 0x1;
        auto response_queue = test_mocks::MockI2CResponseQueue{};
        WHEN("we write a buffer and specify under the message size") {
            writer.transact(
                ADDRESS, 3,
                i2c::messages::MaxMessageBuffer{u8(0x01), u8(0x02), u8(0x03),
                                                u8(0x04), u8(0x05)},
                3, {.token = 651, .is_completed_poll = true}, response_queue);
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                auto compare = i2c::messages::MaxMessageBuffer{
                    u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(5)};

                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write == 3);
                REQUIRE(msg.transaction.bytes_to_read == 3);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
            THEN("the id is passed through") {
                REQUIRE(msg.id.token == 651);
                REQUIRE(msg.id.is_completed_poll == true);
            }
            THEN("the response is present") {
                REQUIRE(msg.response_writer.queue_ref != nullptr);
                REQUIRE(msg.response_writer.writer != nullptr);
                AND_WHEN("we write a response") {
                    auto response_buf = i2c::messages::MaxMessageBuffer{
                        u8(1), u8(2), u8(0), u8(4), u8(10)};
                    static_cast<void>(msg.response_writer.write(
                        i2c::messages::TransactionResponse{
                            .id = {.token = 10, .is_completed_poll = false},
                            .bytes_read = 5,
                            .read_buffer = response_buf}));
                    auto resp = test_mocks::get_response(response_queue);
                    THEN("the response is correct") {
                        REQUIRE(resp.id.token == 10);
                        REQUIRE(resp.id.is_completed_poll == false);
                        REQUIRE(resp.bytes_read == 5);
                        REQUIRE(resp.read_buffer == response_buf);
                    }
                }
            }
        }
        WHEN("we try and write and read a buffer that is too big") {
            auto too_big =
                std::array<uint8_t, i2c::messages::MAX_BUFFER_SIZE>{};
            too_big.fill(0x11);
            writer.transact(ADDRESS, too_big.size(), too_big, too_big.size(),
                            {.token = 651, .is_completed_poll = true},
                            response_queue);
            auto msg = get_message(queue);
            THEN("the transaction size is limited") {
                auto compare = i2c::messages::MaxMessageBuffer{};
                compare.fill(0x11);
                REQUIRE(msg.transaction.write_buffer == compare);
                REQUIRE(msg.transaction.bytes_to_write ==
                        i2c::messages::MAX_BUFFER_SIZE);
                REQUIRE(msg.transaction.bytes_to_read ==
                        i2c::messages::MAX_BUFFER_SIZE);
                REQUIRE(msg.transaction.address == ADDRESS);
            }
        }
    }
}
