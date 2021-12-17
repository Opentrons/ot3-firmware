#pragma once

#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/task_holder.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/linear_motion_system.hpp"

namespace gantry_motion_controller_task {

using MotorControllerType = motion_controller_task::MotorControllerTask<lms::BeltConfig>;

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