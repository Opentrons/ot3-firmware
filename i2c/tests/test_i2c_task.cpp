#include <cstring>
#include <vector>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/simulation/i2c_sim.hpp"
#include "i2c/tests/mock_response_queue.hpp"

#define u8(val) static_cast<uint8_t>(val)

SCENARIO("read and write data to the i2c task") {
    GIVEN("an i2c task and scaffolding") {
        test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
        test_mocks::MockI2CResponseQueue response_queue{};
        i2c::writer::TaskMessage empty_msg{};
        auto writer = i2c::writer::Writer<test_mocks::MockMessageQueue>{};
        writer.set_queue(&i2c_queue);

        auto sim_i2c = i2c::hardware::SimI2C{};

        auto i2c_handler = i2c::tasks::I2CMessageHandler{sim_i2c};

        auto buffer =
            i2c::messages::MaxMessageBuffer{u8(1), u8(2), u8(3), u8(4), u8(5)};
        auto empty_txn = i2c::messages::Transaction{.address = 15,
                                                    .bytes_to_read = 0,
                                                    .bytes_to_write = 0,
                                                    .write_buffer = buffer};
        auto empty_response = i2c::messages::ResponseWriter{};
        auto id = i2c::messages::TransactionIdentifier{
            .token = 25, .is_completed_poll = true};
        auto real_response = i2c::messages::ResponseWriter(response_queue);
        WHEN("handling a fully empty message") {
            auto empty = i2c::writer::TaskMessage{
                i2c::messages::Transact{.transaction = empty_txn,
                                        .id = id,
                                        .response_writer = empty_response}};
            i2c_handler.handle_message(empty);
            THEN("the simulator should not have been used") {
                REQUIRE(sim_i2c.get_transmit_count() == 0);
                REQUIRE(sim_i2c.get_receive_count() == 0);
            }
            // it also shouldn't send a response but the way we test that is by
            // not crashing
        }
        WHEN("handling an empty transaction with response") {
            auto response_only = i2c::writer::TaskMessage{
                i2c::messages::Transact{.transaction = empty_txn,
                                        .id = id,
                                        .response_writer = real_response}};
            i2c_handler.handle_message(response_only);
            THEN("the simulator should not have been used") {
                REQUIRE(sim_i2c.get_transmit_count() == 0);
                REQUIRE(sim_i2c.get_receive_count() == 0);
            }
            THEN(
                "a response should have been enqueued with the mirrored data") {
                auto resp = test_mocks::get_response(response_queue);
                REQUIRE(resp.bytes_read == 0);
                REQUIRE(resp.read_buffer == i2c::messages::MaxMessageBuffer{});
                REQUIRE(resp.id == id);
            }
        }
        WHEN("handling a real transaction with empty response") {
            auto real_txn = empty_txn;
            real_txn.bytes_to_read = 3;
            real_txn.bytes_to_write = 4;
            auto txn_only = i2c::writer::TaskMessage{
                i2c::messages::Transact{.transaction = real_txn,
                                        .id = id,
                                        .response_writer = empty_response}};
            i2c_handler.handle_message(txn_only);
            THEN("the simulator should have been called") {
                REQUIRE(sim_i2c.get_transmit_count() == 1);
                std::vector check{u8(1), u8(2), u8(3), u8(4)};
                REQUIRE(sim_i2c.get_last_transmitted() == check);
                REQUIRE(sim_i2c.get_receive_count() == 1);
                REQUIRE(sim_i2c.get_last_receive_length() == 3);
            }
            // no response aka don't crash
        }
        WHEN("handling a full transaction with response") {
            auto real_txn = empty_txn;
            real_txn.bytes_to_read = 3;
            real_txn.bytes_to_write = 4;
            sim_i2c.set_next_received(
                std::vector{u8(7), u8(9), u8(10), u8(11)});
            auto both = i2c::writer::TaskMessage{
                i2c::messages::Transact{.transaction = real_txn,
                                        .id = id,
                                        .response_writer = real_response}};
            i2c_handler.handle_message(both);
            THEN("the simulator should have been called") {
                REQUIRE(sim_i2c.get_transmit_count() == 1);
                std::vector check{u8(1), u8(2), u8(3), u8(4)};
                REQUIRE(sim_i2c.get_last_transmitted() == check);
                REQUIRE(sim_i2c.get_receive_count() == 1);
                REQUIRE(sim_i2c.get_last_receive_length() == 3);
            }
            THEN("the response should be present and correct") {
                auto resp = test_mocks::get_response(response_queue);
                REQUIRE(resp.bytes_read == 3);
                auto check_buffer = i2c::messages::MaxMessageBuffer{
                    u8(7), u8(9), u8(10), u8(0), u8(0)};
                REQUIRE(resp.read_buffer == check_buffer);
                REQUIRE(resp.id == id);
            }
        }
        WHEN("passing a transaction through a memcpy") {
            i2c::writer::TaskMessage response_copy{};
            // by introducing an anonymous scope, creating a temporary there,
            // and then memcpying into the outer variable we can make sure that
            // the original is cleaned up before we try and use the copy, which
            // should stress test any issues brought up by the memcpy
            {
                auto response_only = i2c::writer::TaskMessage{
                    i2c::messages::Transact{.transaction = empty_txn,
                                            .id = id,
                                            .response_writer = real_response}};

                memcpy(&response_copy, &response_only, sizeof(response_only));
            }
            i2c_handler.handle_message(response_copy);
            THEN(
                "a response should have been enqueued with the mirrored data") {
                auto resp = test_mocks::get_response(response_queue);
                REQUIRE(resp.bytes_read == 0);
                REQUIRE(resp.read_buffer == i2c::messages::MaxMessageBuffer{});
                REQUIRE(resp.id == id);
            }
        }
    }
}
