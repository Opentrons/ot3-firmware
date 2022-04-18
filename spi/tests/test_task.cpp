#include <cstring>
#include <vector>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "spi/core/messages.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"
#include "spi/simulation/spi.hpp"
#include "spi/tests/mock_response_queue.hpp"

#define u8(val) static_cast<uint8_t>(val)

SCENARIO("read and write data to the spi task") {
    GIVEN("a spi task and scaffolding") {
        test_mocks::MockMessageQueue<spi::tasks::TaskMessage> spi_queue{};
        test_mocks::MockSpiResponseQueue response_queue{};
        spi::writer::TaskMessage empty_msg{};
        auto writer = spi::writer::Writer<test_mocks::MockMessageQueue>{};
        writer.set_queue(&spi_queue);

        auto sim_spi = spi::hardware::SimSpiDeviceBase{};

        auto spi_handler = spi::tasks::MessageHandler{sim_spi};

        std::array buffer{u8(1), u8(2), u8(3), u8(4), u8(5)};
        auto empty_txn = spi::messages::Transaction{.txBuffer = buffer};
        auto empty_response = spi::messages::ResponseWriter();
        auto id = spi::messages::TransactionIdentifier{.token = 25};
        auto real_response = spi::messages::ResponseWriter(response_queue);
        WHEN("handling a fully empty message") {
            auto empty = spi::writer::TaskMessage{
                spi::messages::Transact{.id = id,
                                            .transaction = empty_txn,
                                            .response_writer = empty_response}};
            spi_handler.handle_message(empty);
            THEN("the simulator should not have been used") {
                REQUIRE(sim_spi.get_txrx_count() == 0);
            }
            // it also shouldn't send a response but the way we test that is by
            // not crashing
        }
        WHEN("handling an empty transaction with response") {
            auto response_only = spi::writer::TaskMessage{
                spi::messages::Transact{
                    .id = id,
                    .transaction = empty_txn,
                    .response_writer = real_response}};
            spi_handler.handle_message(response_only);
            THEN("the simulator should not have been used") {
                REQUIRE(sim_spi.get_txrx_count() == 0);
            }
            THEN(
                "a response should have been enqueued with the mirrored data") {
                auto resp = test_mocks::get_response(response_queue);
                REQUIRE(resp.rxBuffer == std::array<uint8_t, 5>{});
                REQUIRE(resp.id == id);
            }
        }
        WHEN("handling a real transaction with empty response") {
            auto real_txn = empty_txn;
            auto txn_only = spi::writer::TaskMessage{
                spi::messages::Transact{
                    .id = id,
                    .transaction = real_txn,
                    .response_writer = empty_response}};
            spi_handler.handle_message(txn_only);
            THEN("the simulator should have been called") {
                REQUIRE(sim_spi.get_txrx_count() == 1);
                std::vector check{u8(1), u8(2), u8(3), u8(4)};
                REQUIRE(sim_spi.get_last_transmitted() == check);
                REQUIRE(sim_spi.get_last_received() == check);
            }
            // no response aka don't crash
        }
        WHEN("handling a full transaction with response") {
            auto real_txn = empty_txn;
            sim_spi.set_next_received(
                std::vector{u8(7), u8(9), u8(10), u8(11)});
            auto both = spi::writer::TaskMessage{
                spi::messages::Transact{
                    .id = id,
                    .transaction = real_txn,
                    .response_writer = real_response}};
            spi_handler.handle_message(both);
            THEN("the simulator should have been called") {
                std::vector check{u8(1), u8(2), u8(3), u8(4)};
                REQUIRE(sim_spi.get_last_transmitted() == check);
            }
            THEN("the response should be present and correct") {
                auto resp = test_mocks::get_response(response_queue);
                std::array check_buffer{u8(7), u8(9), u8(10), u8(0), u8(0)};
                REQUIRE(resp.rxBuffer == check_buffer);
                REQUIRE(resp.id == id);
            }
        }
        WHEN("passing a transaction through a memcpy") {
            spi::writer::TaskMessage response_copy{};
            // by introducing an anonymous scope, creating a temporary there,
            // and then memcpying into the outer variable we can make sure that
            // the original is cleaned up before we try and use the copy, which
            // should stress test any issues brought up by the memcpy
            {
                auto response_only = spi::writer::TaskMessage{
                    spi::messages::Transact{.id = id,
                                            .transaction = empty_txn,
                                            .response_writer = real_response}};

                memcpy(&response_copy, &response_only, sizeof(response_only));
            }
            spi_handler.handle_message(response_copy);
            THEN(
                "a response should have been enqueued with the mirrored data") {
                auto resp = test_mocks::get_response(response_queue);
//                REQUIRE(resp.bytes_read == 0);
                REQUIRE(resp.rxBuffer == std::array<uint8_t, 5>{});
                REQUIRE(resp.id == id);
            }
        }
    }
}