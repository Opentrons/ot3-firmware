#pragma once

#include "gantry/core/tasks.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

namespace gantry_move_group_task {

using MoveGroupTaskType =
    move_group_task::MoveGroupTask<gantry_tasks::QueueClient,
                                   gantry_tasks::QueueClient>;

auto start_task(gantry_tasks::QueueClient& client) -> MoveGroupTaskType&;

}  // namespace gantry_move_group_task