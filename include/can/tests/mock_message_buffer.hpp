#pragma once

#include <array>

namespace mock_message_buffer {

/**
 * Mock message buffer for testing reading and writing a single message.
 *
 * @tparam MaxBufferSize
 */
template <std::size_t MaxBufferSize>
struct MockMessageBuffer {

    template <typename Iter, typename Limit>
    MockMessageBuffer(Iter iter, Limit limit)
    : length{limit - iter} {
        std::copy(iter, limit, buff.begin());
    }

    MockMessageBuffer() {
    }

    template <typename Iter, typename Limit>
    auto send(Iter iter, Limit limit, uint32_t timeout) -> std::size_t {
        auto buffer_length = limit - iter;
        std::copy(iter, limit, buff.begin());
        this->timeout = timeout;
        length = buffer_length;
        return buffer_length;
    }

    template <typename Iter, typename Limit>
    auto send_from_isr(Iter iter, Limit limit) -> std::size_t {
        auto buffer_length = limit - iter;
        std::copy(iter, limit, buff.begin());
        length = buffer_length;
        return buffer_length;
    }

    template <typename Iter, typename Limit>
    auto receive(Iter iter, Limit limit, uint32_t timeout) -> std::size_t {
        auto buffer_length = limit - iter;
        auto read_iter = buff.begin();
        for (int i = 0; i < buffer_length; i++) {
            *(iter++) = *(read_iter++);
        }
        this->timeout = timeout;
        return length;
    }

    std::array<uint8_t, MaxBufferSize> buff{};
    long int length = 0;
    uint32_t timeout = 0;
};


} // namespace mock_message_buffer