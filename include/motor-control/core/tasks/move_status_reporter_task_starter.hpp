#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace move_status_reporter_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient,
          lms::MotorMechanicalConfig LmsConfig>
class TaskStarter {
  public:
    using MoveStatusReporterTaskType =
        move_status_reporter_task::MoveStatusReporterTask<
            freertos_message_queue::FreeRTOSMessageQueue, CanClient, LmsConfig>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<
        StackDepth, MoveStatusReporterTaskType, CanClient,
        const lms::LinearMotionSystemConfig<LmsConfig>>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, CanClient& can_client,
               const lms::LinearMotionSystemConfig<LmsConfig>& lms_config)
        -> MoveStatusReporterTaskType& {
        task.start(priority, "move-status", &can_client, &lms_config);
        return task_entry;
    }

  private:
    QueueType queue{};
    MoveStatusReporterTaskType task_entry;
    TaskType task;
};
}  // namespace move_status_reporter_task_starter
