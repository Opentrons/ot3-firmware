#pragma once

#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/task_holder.hpp"

namespace gantry_move_status_reporter_task {


auto constexpr motion_controller_stack_depth = 256;
using MotionControllerTaskType =
    freertos_task::FreeRTOSTask<motion_controller_stack_depth,
                                MotorControllerType>;

/**
 * Start the motion controller task
 *
 * @param driver The controller to use
 * @param all_tasks All the tasks in the system
 * @return The task
 */
auto start_task(MotorControllerType& driver,
                task_holder::TaskHolder& all_tasks) -> MotionControllerTaskType;
}