#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace motion_controller_task_starter {

template <lms::MotorMechanicalConfig LmsConfig, uint32_t StackDepth,
          message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using MotionControllerType = motion_controller::MotionController<LmsConfig>;
    using MotionControllerTaskType =
        motion_controller_task::MotionControllerTask<
            freertos_message_queue::FreeRTOSMessageQueue, LmsConfig, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, MotionControllerTaskType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, MotionControllerType& controller,
               CanClient& can_client) -> MotionControllerTaskType& {
        task.start(priority, "motion-ctrl", &controller, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    MotionControllerTaskType task_entry;
    TaskType task;
};
}  // namespace motion_controller_task_starter
