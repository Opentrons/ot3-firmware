#pragma once

#include <array>
#include <variant>

#include "common/tests/mock_message_queue.hpp"
#include "i2c/core/messages.hpp"
namespace test_mocks {
using MockI2CResponseMessage =
    std::variant<std::monostate, i2c::messages::TransactionResponse>;
using MockI2CResponseQueue =
    test_mocks::MockMessageQueue<MockI2CResponseMessage>;

template <typename Queue>
auto get_response(Queue& queue) -> i2c::messages::TransactionResponse {
    CHECK(queue.has_message());
    test_mocks::MockI2CResponseMessage empty_msg;
    queue.try_read(&empty_msg);
    return std::get<i2c::messages::TransactionResponse>(empty_msg);
}

inline auto dummy_response(
    const i2c::messages::Transact& m,
    const std::array<uint8_t, i2c::messages::MAX_BUFFER_SIZE>& resp = {})
    -> i2c::messages::TransactionResponse {
    return i2c::messages::TransactionResponse{
        .id = m.id,
        .bytes_read = m.transaction.bytes_to_read,
        .read_buffer = resp

    };
}

template <typename Message>
requires std::is_same_v<Message, i2c::messages::SingleRegisterPollRead> ||
    std::is_same_v<Message,
                   i2c::messages::ConfigureSingleRegisterContinuousPolling>
inline auto dummy_single_response(const Message& msg, bool done = false,
                                  const std::array<uint8_t, 5>& resp = {})
    -> i2c::messages::TransactionResponse {
    auto id = msg.id;
    id.is_completed_poll = done;
    return i2c::messages::TransactionResponse{
        .id = id, .bytes_read = msg.first.bytes_to_read, .read_buffer = resp};
}

template <typename Message>
requires std::is_same_v<Message, i2c::messages::MultiRegisterPollRead> ||
    std::is_same_v<Message,
                   i2c::messages::ConfigureMultiRegisterContinuousPolling>
inline auto dummy_multi_response(const Message& msg, std::size_t which,
                                 bool done = false,
                                 const std::array<uint8_t, 5>& resp = {}) {
    auto id = msg.id;
    id.is_completed_poll = done;
    id.transaction_index = which;
    return i2c::messages::TransactionResponse{
        .id = id,
        .bytes_read =
            ((which == 0) ? msg.first.bytes_to_read : msg.second.bytes_to_read),
        .read_buffer = resp};
}

template <typename Message, typename Queue>
inline auto launder_response(Message& msg, Queue& q,
                             const i2c::messages::TransactionResponse& tr)
    -> i2c::messages::TransactionResponse {
    static_cast<void>(msg.response_writer.write(tr));
    return get_response(q);
}

};  // namespace test_mocks
