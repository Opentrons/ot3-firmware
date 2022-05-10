#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_timer.hpp"

namespace central_tasks {

/**
 * Start central tasks.
 */
void start_tasks(can_bus::CanBus& can_bus, can_ids::NodeId id);

/**
 * Access to all tasks not associated with a motor. This will be a singleton.
 */
struct Tasks {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
};

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    freertos_message_queue::FreeRTOSMessageQueue<
        message_writer_task::TaskMessage>* can_writer{nullptr};
};

/**
 * Access to the gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace central_tasks
