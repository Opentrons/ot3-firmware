#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace mock_message_queue {

template <typename Message, std::size_t queue_size = 10>
class MockMessageQueue {
  public:
    explicit MockMessageQueue()
        : queue_data_structure(), message() {}
    MockMessageQueue& operator=(MockMessageQueue&) = delete;
    MockMessageQueue&& operator=(MockMessageQueue&&) = delete;
    MockMessageQueue(MockMessageQueue&) = delete;
    MockMessageQueue(MockMessageQueue&&) = delete;

    ~MockMessageQueue() { }

    auto try_write(const Message& message,
                                 const uint32_t timeout_ticks = 0) -> bool {
        queue_data_structure.push_back(message);
        return true;
    }

    auto try_read(Message* message, uint32_t timeout_ticks = 0) -> bool {
        Message msg = queue_data_structure.back();
        message = &msg;
        return true;
    }

    auto try_write_isr(const Message& message) -> bool {
        return try_write(message);
    }

    auto try_read_isr(Message* message) -> bool {
        return try_read(message);
    }

    auto has_message() const -> bool {
        return queue_data_structure.empty() != 0;
    }

    auto has_message_isr() const -> bool {
        return queue_data_structure.empty() != 0;
    }

    auto peek_isr(Message* message) const -> bool {
        return try_read(message);
    }

    void reset() {
        std::array<uint8_t, queue_size * sizeof(Message)> empty_queue{};
        queue_data_structure = empty_queue;
        message = Message{};
    }

    void get_current_message() { return message; }

    uint8_t get_size() { return queue_data_structure.size(); }

  private:
    std::vector<Message> queue_data_structure;
    Message message;
};
}  // namespace mock_message_queue