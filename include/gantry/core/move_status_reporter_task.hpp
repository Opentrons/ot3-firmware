#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/tasks.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace gantry_move_status_reporter_task {

using MoveStatusReporterTaskType =
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue,
        gantry_tasks::QueueClient>;

auto start_task(gantry_tasks::QueueClient& queueClient)
    -> MoveStatusReporterTaskType&;
}  // namespace gantry_move_status_reporter_task