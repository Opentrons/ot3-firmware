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
class PollerTaskStarter {
  public:
    using PollerTaskType =
        tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                             freertos_timer::FreeRTOSTimer>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<poller::TaskMessage>;
    using I2CWriterType =
        i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PollerTaskType, I2CWriterType>;

    PollerTaskStarter() : task_entry{queue}, task{task_entry} {}
    PollerTaskStarter(const PollerTaskStarter& c) = delete;
    PollerTaskStarter(const PollerTaskStarter&& c) = delete;
    auto operator=(const PollerTaskStarter& c) = delete;
    auto operator=(const PollerTaskStarter&& c) = delete;
    ~PollerTaskStarter() = default;

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
