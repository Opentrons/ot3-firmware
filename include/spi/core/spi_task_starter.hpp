#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/spi.hpp"
#include "spi/core/spi_task.hpp"

namespace spi_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using SPIBaseType = spi::SpiDeviceBase;
    using SPITaskType = spi_task::SPITask<
        freertos_message_queue::FreeRTOSMessageQueue>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        spi_task::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, SPITaskType,
        SPIBaseType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, SPIBaseType& driver)
    -> SPITaskType& {
        task.start(priority, "spi", &driver);
        return task_entry;
    }

  private:
    QueueType queue{};
    SPITaskType task_entry;
    TaskType task;
};

}  // namespace spi_task_starter