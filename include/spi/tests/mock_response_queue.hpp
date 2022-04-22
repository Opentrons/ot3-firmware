#pragma once

#include <array>
#include <variant>

#include "common/tests/mock_message_queue.hpp"
#include "spi/core/messages.hpp"

namespace test_mocks {
using MockSpiResponseMessage =
    std::variant<std::monostate, spi::messages::TransactResponse>;
using MockSpiResponseQueue =
    test_mocks::MockMessageQueue<MockSpiResponseMessage>;

template <typename Queue>
auto get_response(Queue& queue) -> spi::messages::TransactResponse {
    CHECK(queue.has_message());
    test_mocks::MockSpiResponseMessage empty_msg;
    queue.try_read(&empty_msg);
    return std::get<spi::messages::TransactResponse>(empty_msg);
}

inline auto dummy_response(const spi::messages::Transact& m,
                           const std::array<uint8_t, 5>& resp = {})
    -> spi::messages::TransactResponse {
    return spi::messages::TransactResponse{.id = m.id, .rxBuffer = resp};
}

template <typename Message, typename Queue>
inline auto launder_response(Message& msg, Queue& q,
                             const spi::messages::TransactResponse& tr)
    -> spi::messages::TransactResponse {
    msg.response_writer.write(tr);
    return get_response(q);
}

};  // namespace test_mocks