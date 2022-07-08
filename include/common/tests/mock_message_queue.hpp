#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>

#include "common/core/message_queue.hpp"

namespace test_mocks {

template <typename Message, std::size_t queue_size = 10>
class MockMessageQueue {
  public:
    static auto constexpr max_delay = 10000;

    explicit MockMessageQueue() : queue_data_structure() {}
    MockMessageQueue& operator=(MockMessageQueue&) = delete;
    MockMessageQueue&& operator=(MockMessageQueue&&) = delete;
    MockMessageQueue(MockMessageQueue&) = delete;
    MockMessageQueue(MockMessageQueue&&) = delete;

    ~MockMessageQueue() {}

    auto try_write(const Message& message, uint32_t timeout_ticks = 0) -> bool {
        static_cast<void>(timeout_ticks);
        if (get_size() < queue_size) {
            queue_data_structure.push_back(message);
            return true;
        }
        return false;
    }

    template <typename OtherMessage>
    requires ConvertibleMessage<Message, OtherMessage>
    auto try_write(const OtherMessage& message, uint32_t timeout_ticks = 0)
        -> bool {
        Message our_message(message);
        return try_write(our_message, timeout_ticks);
    }

    static auto try_write_static(void* self, auto& om) -> bool {
        auto instance =
            reinterpret_cast<MockMessageQueue<Message, queue_size>*>(self);
        return instance->try_write(om);
    }

    auto try_read(Message* message, uint32_t timeout_ticks = 0) -> bool {
        static_cast<void>(timeout_ticks);
        if (has_message()) {
            *message = queue_data_structure.front();
            queue_data_structure.pop_front();
            return true;
        }
        return false;
    }

    auto try_write_isr(const Message& message) -> bool {
        return try_write(message);
    }

    auto try_read_isr(Message* message) -> bool { return try_read(message); }

    auto has_message() const -> bool {
        return queue_data_structure.empty() != 1;
    }

    auto has_message_isr() const -> bool {
        return queue_data_structure.empty() != 1;
    }

    auto peek_isr(Message* message) const -> bool { return try_read(message); }

    auto peek(Message* message, uint32_t timeout_ticks) const -> bool {
        return try_read(message, timeout_ticks);
    }

    void reset() { queue_data_structure.clear(); }

    uint8_t get_size() { return queue_data_structure.size(); }

  private:
    std::deque<Message> queue_data_structure;
};
}  // namespace test_mocks
