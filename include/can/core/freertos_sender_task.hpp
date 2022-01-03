#pragma once

#include <array>

#include "FreeRTOS.h"
#include "can/core/can_bus.hpp"
#include "can/core/message_core.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"

namespace freertos_sender_task {

struct TaskMessage {
    uint32_t arbitration_id;
    can_messages::ResponseMessageType message;
};

/**
 * Entry point for a CAN sender class.
 * @tparam QueueImpl
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MessageSenderTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;

    /**
     * Constructor
     * @param queue The message queue instance.
     */
    MessageSenderTask(QueueType& queue) : queue{queue} {}

    ~MessageSenderTask() = default;

    MessageSenderTask(const MessageSenderTask&) = delete;
    MessageSenderTask(const MessageSenderTask&&) = delete;
    auto operator=(const MessageSenderTask&) = delete;
    auto operator=(const MessageSenderTask&&) = delete;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(can_bus::CanBus* can) {
        TaskMessage message{};
        while (true) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                auto arbitration_id = message.arbitration_id;
                std::visit(
                    [this, can, arbitration_id](auto m) {
                        this->handle(can, arbitration_id, m);
                    },
                    message.message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    void handle(can_bus::CanBus* can, uint32_t arbitration_id,
                const auto& message) {
        auto length = message.serialize(data.begin(), data.end());
        can->send(arbitration_id, data.begin(), to_canfd_length(length));
    }

    QueueType& queue;
    std::array<uint8_t, message_core::MaxMessageSize> data{};
};

}  // namespace freertos_sender_task