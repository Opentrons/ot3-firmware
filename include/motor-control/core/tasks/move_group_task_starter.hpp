#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

namespace move_group_task_starter {

template <uint32_t StackDepth, motion_controller_task::TaskClient MotionClient,
          message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using MoveGroupTaskType = move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue, MotionClient, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        move_group_task::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, MoveGroupTaskType>;

    TaskStarter() : task_entry{queue, move_group_manager}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, MotionClient& motion_client,
               CanClient& can_client) -> MoveGroupTaskType& {
        task.start(priority, "move-group", &motion_client, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    move_group_task::MoveGroupType move_group_manager{};
    MoveGroupTaskType task_entry;
    TaskType task;
};
}  // namespace move_group_task_starter
