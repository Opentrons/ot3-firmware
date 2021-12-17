#pragma once

#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/task_holder.hpp"

namespace gantry_move_group_task {


auto constexpr move_group_stack_depth = 256;
using MoveGroupTaskType =
    freertos_task::FreeRTOSTask<move_group_stack_depth,
                                move_group_task::MoveGroupTask>;

/**
 * Start the move group task
 *
 * @param all_tasks All the tasks in the system
 * @return The task
 */
auto start_task(task_holder::TaskHolder& all_tasks) -> MoveGroupTaskType;

}