#include <array>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/utils.hpp"
#include "spi/core/writer.hpp"
#include "spi/tests/mock_response_queue.hpp"

#define u8(X) static_cast<uint8_t>(X)

template <typename Queue>
auto get_message(Queue& queue) -> spi::messages::Transact {
    spi::writer::TaskMessage task_msg;
    queue.try_read(&task_msg);
    return std::get<spi::messages::Transact>(task_msg);
}

SCENARIO("Test the spi command queue writer") {
    test_mocks::MockMessageQueue<spi::tasks::TaskMessage> queue{};
    auto response_queue = test_mocks::MockSpiResponseQueue{};
    auto writer = spi::writer::Writer<test_mocks::MockMessageQueue>{};
    writer.set_queue(&queue);
    spi::writer::TaskMessage empty_msg{};

    spi::utils::ChipSelectInterface empty_cs{};
    GIVEN("a write request") {
        constexpr uint8_t TEST_REGISTER = 0x23;
        constexpr uint8_t WRITE_BIT = 0x80;

        WHEN("we write some data to a register") {
            writer.write(TEST_REGISTER, 0xd34db33f, response_queue, empty_cs);
            THEN("the queue should contain one messages") {
                REQUIRE(queue.get_size() == 1);
            }
            auto msg = get_message(queue);
            THEN("the transaction is correct") {
                std::array compare{u8(TEST_REGISTER | WRITE_BIT), u8(0xd3),
                                   u8(0x4d), u8(0xb3), u8(0x3f)};
                REQUIRE(msg.transaction.txBuffer == compare);
                REQUIRE(msg.id.requires_response == true);
                REQUIRE(msg.id.token == TEST_REGISTER);
            }
        }
    }
    GIVEN("a read request") {
        constexpr uint8_t TEST_REGISTER = 0x1;
        WHEN("we receive a read request") {
            writer.read(TEST_REGISTER, 0x0, response_queue, empty_cs);
            THEN("the queue should contain one message") {
                REQUIRE(queue.get_size() == 2);
            }
            auto read_msg = get_message(queue);
            auto response_msg = get_message(queue);
            THEN("the id is correct for the read message") {
                REQUIRE(read_msg.id.requires_response == false);
                REQUIRE(read_msg.id.token == TEST_REGISTER);
            }
            THEN("the id is correct for the response message") {
                REQUIRE(response_msg.id.requires_response == true);
                REQUIRE(response_msg.id.token == TEST_REGISTER);
            }
            THEN("the response is present for the response message") {
                REQUIRE(response_msg.response_writer.queue_ref != nullptr);
                REQUIRE(response_msg.response_writer.writer != nullptr);
                AND_WHEN("we try and write a response") {
                    std::array check_buf{u8(1), u8(2), u8(3), u8(4), u8(5)};
                    response_msg.response_writer.write(
                        spi::messages::TransactResponse{.id = {.token = 25},
                                                        .rxBuffer = check_buf,
                                                        .success = true});
                    auto resp = test_mocks::get_response(response_queue);
                    THEN("the response is correct") {
                        REQUIRE(resp.id.token == 25);
                        REQUIRE(resp.success == true);
                        REQUIRE(resp.rxBuffer == check_buf);
                    }
                }
            }
        }
    }
}