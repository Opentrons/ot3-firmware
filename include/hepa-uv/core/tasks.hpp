#pragma once
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "hepa-uv/core/hepa_task.hpp"
#include "hepa-uv/core/uv_task.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"

namespace hepauv_tasks {

/**
 * Start hepa-uv tasks.
 */
void start_tasks(can::bus::CanBus& can_bus,
                 gpio_drive_hardware::GpioDrivePins& gpio_drive_pins);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);

    void send_hepa_message(const hepa_task::TaskMessage& m);
    void send_uv_message(const hepa_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<hepa_task::TaskMessage>*
        hepa_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<uv_task::TaskMessage>*
        uv_queue{nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};

    hepa_task::HepaTask<freertos_message_queue::FreeRTOSMessageQueue>*
        hepa_task_handler{nullptr};

    uv_task::UVTask<freertos_message_queue::FreeRTOSMessageQueue>*
        uv_task_handler{nullptr};

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

}  // namespace hepauv_tasks