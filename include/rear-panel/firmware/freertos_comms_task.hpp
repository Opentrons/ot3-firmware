/*
 * Interface for the firmware-specifc parts of the host comms task
 */
#pragma once

#include "FreeRTOS.h"
#include "common/core/freertos_message_queue.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/core/tasks/host_comms_task.hpp"
#include "task.h"

namespace host_comms_control_task {
// Function that starts the task
auto start() -> rear_panel_tasks::Task<
    TaskHandle_t, host_comms_task::HostCommTask<
                      freertos_message_queue::FreeRTOSMessageQueue>>;
}  // namespace host_comms_control_task
