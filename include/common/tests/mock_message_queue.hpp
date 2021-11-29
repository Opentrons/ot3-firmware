#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace test_mocks {

template <typename Message, std::size_t queue_size = 10>
class MockMessageQueue {
  public:
    explicit MockMessageQueue() : queue_data_structure() {}
    MockMessageQueue& operator=(MockMessageQueue&) = delete;
    MockMessageQueue&& operator=(MockMessageQueue&&) = delete;
    MockMessageQueue(MockMessageQueue&) = delete;
    MockMessageQueue(MockMessageQueue&&) = delete;

    ~MockMessageQueue() {}

    auto try_write(const Message& message, const uint32_t timeout_ticks = 0)
        -> bool {
        queue_data_structure.push_back(message);
        return true;
    }

    auto try_read(Message* message, uint32_t timeout_ticks = 0) -> bool {
        if (has_message()) {
            *message = queue_data_structure.back();
            queue_data_structure.pop_back();
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

    void reset() { queue_data_structure.clear(); }

    uint8_t get_size() { return queue_data_structure.size(); }

  private:
    std::vector<Message> queue_data_structure;
};
}  // namespace test_mocks
