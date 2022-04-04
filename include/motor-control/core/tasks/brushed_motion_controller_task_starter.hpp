#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"

namespace brushed_motion_controller_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using MotorType = motor_hardware::BrushedMotorHardwareIface;
    using MotorTaskType = brushed_motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motion_controller_task::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, MotorTaskType,
                                                 MotorType, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, MotorType& motor, CanClient& can_client)
        -> MotorTaskType& {
        task.start(priority, "brushed motor", &motor, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    MotorTaskType task_entry;
    TaskType task;
};
}  // namespace brushed_motion_controller_task_starter