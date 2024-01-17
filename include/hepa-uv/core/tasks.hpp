#pragma once
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"

namespace hepauv_tasks {

/**
 * Start hepa-uv tasks.
 */
void start_tasks(can::bus::CanBus& can_bus);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
};

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_all_tasks() -> AllTask&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_main_queues() -> QueueClient&;

[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace hepauv_tasks
