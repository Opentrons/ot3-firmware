#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "pipettes/core/tasks.hpp"

namespace pipettes_move_status_reporter_task {

using MoveStatusReporterTaskType =
    move_status_reporter_task::MoveStatusReporterTask<
        pipettes_tasks::QueueClient>;

auto start_task(pipettes_tasks::QueueClient& queueClient)
    -> MoveStatusReporterTaskType&;
}  // namespace pipettes_move_status_reporter_task