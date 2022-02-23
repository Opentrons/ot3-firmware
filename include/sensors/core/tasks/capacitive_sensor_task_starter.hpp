#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/core/utils.hpp"

namespace capacitive_sensor_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using I2CWriterType =
    i2c_writer::I2CWriter<freertos_message_queue::FreeRTOSMessageQueue>;
    using CapacitiveTaskType =
    capacitive_task::CapacitiveTask<freertos_message_queue::FreeRTOSMessageQueue,
        I2CWriterType, CanClient>;
    using QueueType =
    freertos_message_queue::FreeRTOSMessageQueue<sensor_task_utils::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, CapacitiveTaskType,
        I2CWriterType, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer, CanClient& can_client)
    -> CapacitiveTaskType& {
        task.start(priority, "capacitive", &writer, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    CapacitiveTaskType task_entry;
    TaskType task;
};

}  // namespace capacitive_sensor_task_starter