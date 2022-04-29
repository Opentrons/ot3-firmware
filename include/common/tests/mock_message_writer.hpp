#pragma once

#include "can/core/message_core.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"

namespace mock_message_writer {

struct TaskMessage {
    uint32_t arbitration_id;
    can_messages::ResponseMessageType message;
};

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MockMessageWriter {
  public:
    using QueueType = QueueImpl<TaskMessage>;

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    void send_can_message(can_ids::NodeId, ResponseMessage&& message) {
        TaskMessage task_msg{.arbitration_id = 0x1, .message = message};
        queue->try_write(task_msg);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    QueueType* queue{nullptr};
};
}  // namespace mock_message_writer
