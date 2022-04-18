#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "head/core/adc.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/core/tasks/presence_sensing_driver_task.hpp"

namespace presence_sensing_driver_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using PresenceSensingDriverTaskType =
        presence_sensing_driver_task::PresenceSensingDriverTask<
            freertos_message_queue::FreeRTOSMessageQueue, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        presence_sensing_driver_task::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PresenceSensingDriverTaskType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(
        uint32_t priority,
        presence_sensing_driver::PresenceSensingDriver& presence_sensing_driver,
        CanClient& can_client) -> PresenceSensingDriverTaskType& {
        task.start(priority, "presence-sensing", &presence_sensing_driver,
                   &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    PresenceSensingDriverTaskType task_entry;
    TaskType task;
};
}  // namespace presence_sensing_driver_task_starter
