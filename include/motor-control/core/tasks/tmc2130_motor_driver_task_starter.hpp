#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"

namespace tmc2130_motor_driver_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using MotorDriverTaskType = motor_driver_task::TMC2130MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, MotorDriverTaskType,
                                    CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, MotorDriverType& driver,
               CanClient& can_client) -> MotorDriverTaskType& {
        task.start(priority, "motor-driver", &driver, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    MotorDriverTaskType task_entry;
    TaskType task;
};
}  // namespace motor_driver_task_starter