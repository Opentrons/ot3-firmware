#pragma once

#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"

namespace i2c {

namespace task_starters {
template <uint32_t StackDepth>
class I2CTaskStarter {
  public:
    using I2CBaseType = hardware::I2CDeviceBase;
    using I2CTaskType =
        tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<writer::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, I2CTaskType, I2CBaseType>;

    I2CTaskStarter() : task_entry{queue}, task{task_entry} {}
    I2CTaskStarter(const I2CTaskStarter& c) = delete;
    I2CTaskStarter(const I2CTaskStarter&& c) = delete;
    auto operator=(const I2CTaskStarter& c) = delete;
    auto operator=(const I2CTaskStarter&& c) = delete;
    ~I2CTaskStarter() = default;

    auto start(uint32_t priority, hardware::I2CDeviceBase& driver)
        -> I2CTaskType& {
        task.start(priority, "i2c", &driver);
        return task_entry;
    }

  private:
    QueueType queue{};
    I2CTaskType task_entry;
    TaskType task;
};
};  // namespace task_starters
}  // namespace i2c
