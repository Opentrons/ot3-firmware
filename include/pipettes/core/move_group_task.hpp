#pragma once

#include "motor-control/core/tasks/move_group_task.hpp"
#include "pipettes/core/tasks.hpp"

namespace pipettes_move_group_task {

using MoveGroupTaskType =
    move_group_task::MoveGroupTask<pipettes_tasks::QueueClient,
                                   pipettes_tasks::QueueClient>;

auto start_task(pipettes_tasks::QueueClient& client) -> MoveGroupTaskType&;

}  // namespace pipettes_move_group_task