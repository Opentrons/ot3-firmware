#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class EnvironmentalSensorTaskStarter {
  public:
    using I2CWriterType =
        i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
    using EnvironmentTaskType =
        EnvironmentSensorTask<freertos_message_queue::FreeRTOSMessageQueue,
                              I2CWriterType, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<utils::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, EnvironmentTaskType>;

    EnvironmentalSensorTaskStarter() : task_entry{queue}, task{task_entry} {}
    EnvironmentalSensorTaskStarter(const EnvironmentalSensorTaskStarter& c) =
        delete;
    EnvironmentalSensorTaskStarter(const EnvironmentalSensorTaskStarter&& c) =
        delete;
    auto operator=(const EnvironmentalSensorTaskStarter& c) = delete;
    auto operator=(const EnvironmentalSensorTaskStarter&& c) = delete;
    ~EnvironmentalSensorTaskStarter() = default;

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
