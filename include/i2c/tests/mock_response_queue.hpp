#pragma once

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

};  // namespace test_mocks
