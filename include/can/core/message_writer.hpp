#pragma once

#include <array>

#include "arbitration_id.hpp"
#include "can/core/freertos_sender_task.hpp"
#include "can/core/ids.hpp"
#include "can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/synchronization.hpp"
#include "message_core.hpp"
#include "types.h"

namespace can_message_writer {

class MessageWriter {
  public:
    explicit MessageWriter(can_bus::CanBus& writer, can_ids::NodeId node_id)
        : writer{writer}, node_id(node_id) {}
    ~MessageWriter() = default;
    MessageWriter(const MessageWriter&) = delete;
    MessageWriter(MessageWriter&&) = delete;
    auto operator=(const MessageWriter&) -> MessageWriter& = delete;
    auto operator=(MessageWriter&&) -> MessageWriter&& = delete;

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    void write(can_ids::NodeId node, ResponseMessage& message) {
        auto lock = synchronization::Lock{mutex};
        arbitration_id.message_id(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.function_code(can_ids::FunctionCode::network_management);
        arbitration_id.node_id(node);
        arbitration_id.originating_node_id(node_id);
        auto length = message.serialize(buffer.begin(), buffer.end());
        writer.send(arbitration_id, buffer.data(), to_canfd_length(length));
    }

  private:
    can_bus::CanBus& writer;
    freertos_synchronization::FreeRTOSMutex mutex{};
    can_arbitration_id::ArbitrationId arbitration_id{};
    std::array<uint8_t, message_core::MaxMessageSize> buffer{};
    can_ids::NodeId node_id;
};

using namespace freertos_sender_task;

class MessageWriter2 {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;

    explicit MessageWriter2(QueueType& writer, can_ids::NodeId node_id)
        : writer{writer}, node_id(node_id) {}

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    void write(can_ids::NodeId node, ResponseMessage& message) {
        auto task_message = TaskMessage{};
        auto arbitration_id = can_arbitration_id::ArbitrationId{};
        arbitration_id.message_id(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.function_code(can_ids::FunctionCode::network_management);
        arbitration_id.node_id(node);

        task_message.arbitration_id = arbitration_id;

        message.set_node_id(node_id);
        auto length = message.serialize(task_message.data.begin(),
                                        task_message.data.end());
        task_message.data_length = to_canfd_length(length);
        writer.try_write(task_message);
    }

  private:
    QueueType& writer;
    can_ids::NodeId node_id;
};
}  // namespace can_message_writer
