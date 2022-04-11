#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using I2CWriterType =
        i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
    using EnvironmentTaskType = environment_sensor_task::EnvironmentSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue, I2CWriterType, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<utils::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, EnvironmentTaskType,
                                    I2CWriterType, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer, CanClient& can_client)
        -> EnvironmentTaskType& {
        task.start(priority, "humidity", &writer, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    EnvironmentTaskType task_entry;
    TaskType task;
};
};  // namespace tasks
}  // namespace sensors
