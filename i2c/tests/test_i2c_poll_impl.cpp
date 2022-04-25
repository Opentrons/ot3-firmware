#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "common/tests/mock_timer.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/poller_impl.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

template <typename M>
int get_period(const M& message) {
    return message.delay_ms;
}

template <>
int get_period(const std::monostate& _) {
    static_cast<void>(_);
    return 0;
}

template <>
int get_period(const i2c::messages::TransactionResponse& _) {
    static_cast<void>(_);
    return 0;
}

template <typename M>
uint16_t get_address(const M& message) {
    return message.first.address;
}

template <>
uint16_t get_address(const std::monostate& _) {
    static_cast<void>(_);
    return 0;
}

template <>
uint16_t get_address(const i2c::messages::TransactionResponse& _) {
    static_cast<void>(_);
    return 0;
}

template <typename M>
uint32_t get_poll_id(const M& message) {
    return message.id.token;
}

uint32_t get_poll_id(const std::monostate& _) {
    static_cast<void>(_);
    return 0;
}

uint32_t get_poll_id(const i2c::messages::TransactionResponse& _) {
    static_cast<void>(_);
    return 0;
}

SCENARIO("test poll management") {
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
    auto& poller = poll_handler.continuous_polls;
    GIVEN("continuous poller with no ongoing polling") {
        WHEN("starting a new poll") {
            auto which_msg = GENERATE(
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    i2c::messages::ConfigureSingleRegisterContinuousPolling{
                        .delay_ms = 100,
                        .first = {.address = 0x1234,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,
                                  .write_buffer{}},
                        .id = {.token = 12314,
                               .is_completed_poll = false,
                               .transaction_index = 0}}},
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::ConfigureMultiRegisterContinuousPolling>,
                    i2c::messages::ConfigureMultiRegisterContinuousPolling{
                        .delay_ms = 101,
                        .first = {.address = 0x1235,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer{}},
                        .second = {.address = 0x1235,
                                   .bytes_to_read = 0,
                                   .bytes_to_write = 0,

                                   .write_buffer{}},
                        .id = {.token = 12315,
                               .is_completed_poll = false,
                               .transaction_index = 0}}});
            poll_handler.handle_message(which_msg);
            THEN("the first poll slot should be used") {
                REQUIRE(poller.polls[0].transactions[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   which_msg));
                REQUIRE(
                    poller.polls[0].timer.get_period() ==
                    static_cast<uint32_t>(std::visit(
                        [](auto& msg) { return get_period(msg); }, which_msg)));
                REQUIRE(poller.polls[0].timer.is_running());
                REQUIRE(poller.polls[0].id.token ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   which_msg));
            }
            THEN("no poll slot but the first should be used") {
                auto poll_iter = poller.polls.begin();
                poll_iter++;
                for (; poll_iter != poller.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->transactions[0].address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->id.token == 0);
                }
            }
        }
    }
    GIVEN("an id with a poll active") {
        auto original_message = GENERATE(
            i2c::poller::TaskMessage{
                std::in_place_type<
                    i2c::messages::ConfigureSingleRegisterContinuousPolling>,
                i2c::messages::ConfigureSingleRegisterContinuousPolling{
                    .delay_ms = 100,
                    .first = {.address = 0x1234,
                              .bytes_to_read = 0,
                              .bytes_to_write = 0,

                              .write_buffer = {}},
                    .id = {.token = 12345,
                           .is_completed_poll = false,
                           .transaction_index = 0}}},
            i2c::poller::TaskMessage{
                std::in_place_type<
                    i2c::messages::ConfigureMultiRegisterContinuousPolling>,
                i2c::messages::ConfigureMultiRegisterContinuousPolling{
                    .delay_ms = 101,
                    .first = {.address = 0x1235,
                              .bytes_to_read = 0,
                              .bytes_to_write = 0,

                              .write_buffer = {}},
                    .second = {.address = 0x1236,
                               .bytes_to_read = 0,
                               .bytes_to_write = 0,

                               .write_buffer = {}},
                    .id = {.token = 12345,
                           .is_completed_poll = false,
                           .transaction_index = 0}}});
        poll_handler.handle_message(original_message);
        CHECK(poller.polls[0].transactions[0].address ==
              std::visit([](auto& msg) { return get_address(msg); },
                         original_message));
        CHECK(
            poller.polls[0].timer.get_period() ==
            static_cast<uint32_t>(std::visit(
                [](auto& msg) { return get_period(msg); }, original_message)));
        CHECK(poller.polls[0].timer.is_running());
        CHECK(poller.polls[0].id.token ==
              std::visit([](auto& msg) { return get_poll_id(msg); },
                         original_message));

        WHEN("sending a message to the same id") {
            auto new_message = GENERATE(

                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    i2c::messages::ConfigureSingleRegisterContinuousPolling{
                        .delay_ms = 50,
                        .first = {.address = 0x1233,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer = {}},
                        .id = {.token = 12345,
                               .is_completed_poll = false,
                               .transaction_index = 0}}},
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::ConfigureMultiRegisterContinuousPolling>,
                    i2c::messages::ConfigureMultiRegisterContinuousPolling{
                        .delay_ms = 49,
                        .first = {.address = 0x1200,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer = {}},
                        .second = {.address = 0x1200,
                                   .bytes_to_read = 0,
                                   .bytes_to_write = 0,

                                   .write_buffer = {}},
                        .id = {.token = 12345,
                               .is_completed_poll = false,
                               .transaction_index = 0}}});
            poll_handler.handle_message(new_message);
            THEN("the new message should overwrite the old one") {
                REQUIRE(poller.polls[0].transactions[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   new_message));
                REQUIRE(poller.polls[0].timer.get_period() ==
                        static_cast<uint32_t>(std::visit(
                            [](auto& msg) { return get_period(msg); },
                            new_message)));
                REQUIRE(poller.polls[0].timer.is_running());
                REQUIRE(poller.polls[0].id.token ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   new_message));
            }
            THEN("no poll slot but the first should be used") {
                auto poll_iter = poller.polls.begin();
                poll_iter++;
                for (; poll_iter != poller.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->transactions[0].address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->id.token == 0);
                }
            }
        }
        WHEN(
            "sending a message to the same poll id and address with period 0") {
            auto new_message = GENERATE(

                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    i2c::messages::ConfigureSingleRegisterContinuousPolling{
                        .delay_ms = 0,
                        .first = {.address = 0x1233,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,
                                  .write_buffer = {}},
                        .id = {.token = 12345,
                               .is_completed_poll = false,
                               .transaction_index = 0}}},
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::ConfigureMultiRegisterContinuousPolling>,
                    i2c::messages::ConfigureMultiRegisterContinuousPolling{
                        .delay_ms = 0,
                        .first = {.address = 0x1200,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer = {}},
                        .second = {.address = 0x1200,
                                   .bytes_to_read = 0,
                                   .bytes_to_write = 0,
                                   .write_buffer = {}},
                        .id = {.token = 12345,
                               .is_completed_poll = false,
                               .transaction_index = 0}}});
            poll_handler.handle_message(new_message);
            THEN("the polling should stop and the slot should be freed") {
                REQUIRE(poller.polls[0].transactions[0].address == 0);
                REQUIRE(poller.polls[0].id.token == 0);
                REQUIRE(!poller.polls[0].timer.is_running());
            }
        }
        WHEN("sending a message to a different id") {
            auto new_message = GENERATE(
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::
                            ConfigureSingleRegisterContinuousPolling>,
                    i2c::messages::ConfigureSingleRegisterContinuousPolling{
                        .delay_ms = 782,
                        .first = {.address = 0x1233,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer = {}},
                        .id = {.token = 1200,
                               .is_completed_poll = false,
                               .transaction_index = 0}}},
                i2c::poller::TaskMessage{
                    std::in_place_type<
                        i2c::messages::ConfigureMultiRegisterContinuousPolling>,
                    i2c::messages::ConfigureMultiRegisterContinuousPolling{
                        .delay_ms = 200,
                        .first = {.address = 0x1200,
                                  .bytes_to_read = 0,
                                  .bytes_to_write = 0,

                                  .write_buffer = {}},
                        .second = {.address = 0x1200,
                                   .bytes_to_read = 0,
                                   .bytes_to_write = 0,

                                   .write_buffer = {}},
                        .id = {.token = 64,
                               .is_completed_poll = false,
                               .transaction_index = 0}}});
            poll_handler.handle_message(new_message);
            THEN("the old message should still be in place") {
                REQUIRE(poller.polls[0].transactions[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   original_message));
                REQUIRE(poller.polls[0].timer.get_period() ==
                        static_cast<uint32_t>(std::visit(
                            [](auto& msg) { return get_period(msg); },
                            original_message)));
                REQUIRE(poller.polls[0].timer.is_running());
                REQUIRE(poller.polls[0].id.token ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   original_message));
            }
            THEN("the new message should be in a new slot") {
                REQUIRE(poller.polls[1].transactions[0].address ==
                        std::visit([](auto& msg) { return get_address(msg); },
                                   new_message));
                REQUIRE(poller.polls[1].timer.get_period() ==
                        static_cast<uint32_t>(std::visit(
                            [](auto& msg) { return get_period(msg); },
                            new_message)));
                REQUIRE(poller.polls[1].timer.is_running());
                REQUIRE(poller.polls[1].id.token ==
                        std::visit([](auto& msg) { return get_poll_id(msg); },
                                   new_message));
            }
            THEN("no poll slot but the first two should be used") {
                auto poll_iter = poller.polls.begin();
                poll_iter++;
                poll_iter++;
                for (; poll_iter != poller.polls.end(); poll_iter++) {
                    REQUIRE(poll_iter->transactions[0].address == 0);
                    REQUIRE(!poll_iter->timer.is_running());
                    REQUIRE(poll_iter->id.token == 0);
                }
            }
        }
    }
}
