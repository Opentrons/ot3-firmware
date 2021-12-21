#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace move_status_reporter_task {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

/**
 * The handler of move status messages
 */
template <message_writer_task::TaskClient CanClient>
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler(CanClient& can_client) : can_client{can_client} {}
    ~MoveStatusMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        can_messages::MoveCompleted msg = {
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .current_position = message.current_position,
            .ack_id =
                static_cast<uint8_t>(motor_messages::AckMessageId::complete),
        };
        can_client.send_can_message(can_ids::NodeId::host, msg);
    }

  private:
    CanClient& can_client;
};

/**
 * The task type.
 */
template <message_writer_task::TaskClient CanClient>
class MoveStatusReporterTask {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;
    MoveStatusReporterTask(QueueType& queue) : queue{queue} {}
    ~MoveStatusReporterTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(CanClient* can_client) {
        auto handler = MoveStatusMessageHandler{*can_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_move_status_reporter_queue(m)};
};

}  // namespace move_status_reporter_task