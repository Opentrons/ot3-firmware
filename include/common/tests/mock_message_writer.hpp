#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/message_core.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"

namespace mock_message_writer {

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<can::message_writer_task::TaskMessage>,
                      can::message_writer_task::TaskMessage>
class MockMessageWriter {
  public:
    using QueueType = QueueImpl<can::message_writer_task::TaskMessage>;

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <can::message_core::CanResponseMessage ResponseMessage>
    auto send_can_message(can::ids::NodeId, ResponseMessage&& message) -> bool {
        can::message_writer_task::TaskMessage task_msg{.arbitration_id = 0x1,
                                                       .message = message};
        return queue->try_write(task_msg);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    QueueType* queue{nullptr};
};
}  // namespace mock_message_writer
