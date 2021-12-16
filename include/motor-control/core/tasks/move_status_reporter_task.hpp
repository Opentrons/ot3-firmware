#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue_poller.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace move_status_reporter_task {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

/**
 * The handler of move status messages
 */
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler() = default;
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
};

/**
 * The task type.
 */
using MoveStatusReporterTask =
    freertos_message_queue_poller::FreeRTOSMessageQueuePoller<
        TaskMessage, MoveStatusMessageHandler>;

/**
 * Concept describing a class that can message this task.
 * @tparam TaskHolder
 */
template <typename TaskHolder>
concept TaskClient = requires(TaskHolder holder, const TaskMessage& m) {
    {holder.send_move_status_reporter_queue(m)};
};

}  // namespace move_status_reporter_task