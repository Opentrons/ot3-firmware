#pragma once

#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/writer.hpp"

namespace i2c {

namespace task_starters {

template <uint32_t StackDepth>
class TaskStarter {
  public:
    using PollerTaskType =
        tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                             freertos_timer::FreeRTOSTimer>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<poller::TaskMessage>;
    using I2CWriterType = i2c_writer::I2CWriter<
        freertos_message_queue::FreeRTOSMessageQueue,
        freertos_message_queue::FreeRTOSMessageQueue<poller::TaskMessage>>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PollerTaskType, I2CWriterType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer) -> PollerTaskType& {
        task.start(priority, "i2c-poll", &writer);
        return task_entry;
    }

  private:
    QueueType queue{};
    PollerTaskType task_entry;
    TaskType task;
};
};  // namespace task_starters
}  // namespace i2c
