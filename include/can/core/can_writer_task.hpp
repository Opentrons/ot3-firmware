#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_core.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"

namespace can::message_writer_task {

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
class MessageWriterTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;

    /**
     * Constructor
     * @param queue The message queue instance.
     */
    MessageWriterTask(QueueType& queue) : queue{queue} {}
    MessageWriterTask(const MessageWriterTask&) = delete;
    MessageWriterTask(const MessageWriterTask&&) = delete;
    auto operator=(const MessageWriterTask&) = delete;
    auto operator=(const MessageWriterTask&&) = delete;

    ~MessageWriterTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(can_bus::CanBus* can) {
        TaskMessage message{};
        while (true) {
            if (queue.try_read(&message, queue.max_delay)) {
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

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, can_ids::NodeId node_id,
                              const can_messages::ResponseMessageType& m) {
    {client.send_can_message(node_id, m)};
};

}  // namespace message_writer_task