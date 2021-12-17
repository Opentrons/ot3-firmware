#pragma once

#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/task_holder.hpp"
#include "motor-control/core/motor_driver.hpp"

namespace gantry_motor_driver_task {

auto constexpr motor_driver_stack_depth = 256;
using MotorDriverTaskType =
    freertos_task::FreeRTOSTask<motor_driver_stack_depth,
                                motor_driver_task::MotorDriverTask>;

/**
 * Start the motor driver task
 *
 * @param driver The driver to use
 * @param all_tasks All the tasks in the system
 * @return The task
 */
auto start_task(motor_driver::MotorDriver& driver,
                task_holder::TaskHolder& all_tasks) -> MotorDriverTaskType;

}