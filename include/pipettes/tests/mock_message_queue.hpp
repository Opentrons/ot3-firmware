namespace mock_message_queue {

struct MockMessageQueue {
    explicit MockMessageQueue(uint8_t notification_bit, const char* name)
        : MockMessageQueue(notification_bit) {}
    explicit MockMessageQueue(uint8_t notification_bit)
        : sent_bit(notification_bit) {}
    MockMessageQueue& operator=(MockMessageQueue&) = delete;
    MockMessageQueue&& operator=(MockMessageQueue&&) = delete;
    MockMessageQueue(MockMessageQueue&) = delete;
    MockMessageQueue(MockMessageQueue&&) = delete;

    ~MockMessageQueue() { vQueueDelete(queue); }

    [[nodiscard]] auto try_write(const Message& message,
                                 const uint32_t timeout_ticks = 0) -> bool {
        return sent;
    }

    auto try_read(Message* message, uint32_t timeout_ticks = 0) -> bool {
        message = queue_data_structure.back();
        return true;
    }

    [[nodiscard]] auto try_write_isr(const Message& message) -> bool {
        queue_data_structure.push_back(message);
        return true;
    }

    auto try_read_isr(Message* message) const -> bool {
        return try_read(message);
    }

    [[nodiscard]] auto has_message() const -> bool {
        return queue_data_structure.empty() != 0;
    }

    [[nodiscard]] auto has_message_isr() const -> bool {
        return queue_data_structure.empty() != 0;
    }

    [[nodiscard]] auto peek_isr(Message* message) const -> bool {
        return try_read(message);
    }

    void reset() {
        std::array<uint8_t, queue_size * sizeof(Message)> empty_queue{};
        queue_data_structure = empty_queue;
        message = Message{};
    }

    void get_current_message() { return message; }

  private:
    std::array<uint8_t, queue_size * sizeof(Message)> queue_data_structure{};
    uint8_t sent_bit;
    Message message;
};
}  // namespace mock_message_queue