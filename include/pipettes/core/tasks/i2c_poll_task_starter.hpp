#pragma once

#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/tasks/i2c_poll_task.hpp"

namespace i2c_poll_task_starter {

template <uint32_t StackDepth>
class TaskStarter {
  public:
    using PollTaskType = i2c_poller_task::I2CPollingTask<
        freertos_message_queue::FreeRTOSMessageQueue,
        freertos_timer::FreeRTOSTimer>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<i2c_poller::TaskMessage>;
    using I2CWriterType =
        i2c_writer::I2CWriter<freertos_message_queue::FreeRTOSMessageQueue>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PollTaskType, I2CWriterType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer) -> PollTaskType& {
        task.start(priority, "i2c-poll", &writer);
        return task_entry;
    }

  private:
    QueueType queue{};
    PollTaskType task_entry;
    TaskType task;
};

}  // namespace i2c_poll_task_starter
