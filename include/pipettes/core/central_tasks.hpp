#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_timer.hpp"

/**
 * Central Tasks
 *
 * These tasks will start the CAN bus message writer and reader task.
 */
namespace central_tasks {

/**
 * Start central tasks.
 */
void start_tasks(can::bus::CanBus& can_bus, can::ids::NodeId id);

/**
 * Access to central CAN bus tasks. This will be a singleton.
 */
struct Tasks {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
};

/**
 * Access to the can writer message queue.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    freertos_message_queue::FreeRTOSMessageQueue<
        can::message_writer_task::TaskMessage>* can_writer{nullptr};
};

/**
 * Access to the central tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace central_tasks
