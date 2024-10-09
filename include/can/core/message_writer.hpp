#pragma once

#include <array>

#include "arbitration_id.hpp"
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "message_core.hpp"
#include "types.h"

namespace can::message_writer {

/**
 * A helper class for writing CAN messages into the CAN tests message queue.
 */
class MessageWriter {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        can::message_writer_task::TaskMessage>;

    explicit MessageWriter(can::ids::NodeId node_id) : node_id(node_id) {}

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    auto send_can_message(can::ids::NodeId node, ResponseMessage&& message)
        -> bool {
        auto arbitration_id = can::arbitration_id::ArbitrationId{};
        auto task_message = can::message_writer_task::TaskMessage{};

        arbitration_id.message_id(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.function_code(
            can::ids::FunctionCode::network_management);
        arbitration_id.node_id(node);
        arbitration_id.originating_node_id(node_id);
        task_message.arbitration_id = arbitration_id;
        task_message.message = message;
        return queue->try_write(task_message);
    }

    void set_queue(QueueType* q) { queue = q; }
    void set_node_id(can::ids::NodeId id) { node_id = id; }

  private:
    can::ids::NodeId node_id;
    QueueType* queue{nullptr};
};
}  // namespace can::message_writer
