#pragma once

#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "pipettes/core/tasks.hpp"

namespace pipettes_motor_driver_task {

using MotorDriverTaskType =
    motor_driver_task::MotorDriverTask<pipettes_tasks::QueueClient>;

auto start_task(motor_driver::MotorDriver& driver,
                pipettes_tasks::QueueClient& client) -> MotorDriverTaskType&;

}  // namespace pipettes_motor_driver_task