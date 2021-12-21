#include "pipettes/core/motor_driver_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"

using namespace pipettes_motor_driver_task;

static auto queue = freertos_message_queue::FreeRTOSMessageQueue<
    motor_driver_task::TaskMessage>{};

static auto task_entry = MotorDriverTaskType{queue};

static auto task =
    freertos_task::FreeRTOSTask<512, MotorDriverTaskType,
                                motor_driver::MotorDriver,
                                pipettes_tasks::QueueClient>{task_entry};

auto pipettes_motor_driver_task::start_task(motor_driver::MotorDriver &driver,
                                            pipettes_tasks::QueueClient &client)
    -> MotorDriverTaskType & {
    task.start(5, "motor driver task", &driver, &client);
    return task_entry;
}