#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "common/core/freertos_message_queue.hpp"

namespace move_status_reporter_task {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

/**
 * The handler of move status messages
 */
template <typename AllTasks>
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler(AllTasks& all_tasks): all_tasks{all_tasks} {}
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
        //        message_writer.write(NodeId::host, msg);
    }

  private:
    AllTasks& all_tasks;
};

/**
 * The task type.
 */
template <typename AllTasks>
class MoveStatusReporterTask {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;
    MoveStatusReporterTask(QueueType& queue) : queue{queue} {}
    ~MoveStatusReporterTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(AllTasks* all_tasks) {
        auto handler = MoveStatusMessageHandler{*all_tasks};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                handler.handle_message(message);
            }
        }
    }
  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client holder, const TaskMessage& m) {
    {holder.send_move_status_reporter_queue(m)};
};

}  // namespace move_status_reporter_task