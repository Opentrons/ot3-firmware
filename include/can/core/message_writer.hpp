#pragma once

#include <array>

#include "arbitration_id.hpp"
#include "can/core/freertos_sender_task.hpp"
#include "can/core/ids.hpp"
#include "can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "message_core.hpp"
#include "types.h"

namespace can_message_writer {

using namespace freertos_sender_task;

/**
 * A helper class for writing CAN messages into the CAN tests message queue.
 *
 * This class is not thread safe. One instance per task!
 */
class MessageWriter {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;

    explicit MessageWriter(QueueType& queue, can_ids::NodeId node_id)
        : queue{queue}, node_id(node_id) {}

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    void write(can_ids::NodeId node, ResponseMessage& message) {
        arbitration_id.message_id(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.function_code(can_ids::FunctionCode::network_management);
        arbitration_id.node_id(node);
        arbitration_id.originating_node_id(node_id);
        task_message.arbitration_id = arbitration_id;
        task_message.message = message;
        queue.try_write(task_message);
    }

  private:
    QueueType& queue;
    can_ids::NodeId node_id;
    can_arbitration_id::ArbitrationId arbitration_id{};
    TaskMessage task_message{};
};
}  // namespace can_message_writer
