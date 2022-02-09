#pragma once

#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/i2c.hpp"
#include "pipettes/core/i2c_writer.hpp"

namespace i2c_task_starter {

template <uint32_t StackDepth>
class TaskStarter {
  public:
    using I2CBaseType = i2c::I2CDeviceBase;
    using I2CTaskType =
        i2c_task::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<i2c_writer::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, I2CTaskType, I2CBaseType>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, i2c::I2CDeviceBase& driver) -> I2CTaskType& {
        task.start(priority, "i2c", &driver);
        return task_entry;
    }

  private:
    QueueType queue{};
    I2CTaskType task_entry;
    TaskType task;
};

}  // namespace i2c_task_starter