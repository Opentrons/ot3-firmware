#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"

namespace spi {

namespace task_starters {
template <uint32_t StackDepth>
class TaskStarter {
  public:
    using SpiBaseType = spi::hardware::SpiDeviceBase;
    using SpiTaskType =
        spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, SpiTaskType, SpiBaseType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, SpiBaseType& driver) -> SpiTaskType& {
        task.start(priority, "spi", &driver);
        return task_entry;
    }

  private:
    QueueType queue{};
    SpiTaskType task_entry;
    TaskType task;
};
}  // namespace task_starters

}  // namespace spi