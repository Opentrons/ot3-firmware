#pragma once

#include "gantry/core/tasks.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"

namespace gantry_motor_driver_task {

using MotorDriverTaskType = motor_driver_task::MotorDriverTask<
    freertos_message_queue::FreeRTOSMessageQueue, gantry_tasks::QueueClient>;

auto start_task(motor_driver::MotorDriver& driver,
                gantry_tasks::QueueClient& client) -> MotorDriverTaskType&;

}  // namespace gantry_motor_driver_task