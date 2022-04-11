#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

namespace pressure_sensor_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using I2CWriterType =
        i2c_writer::I2CWriter<freertos_message_queue::FreeRTOSMessageQueue>;
    using I2CPollerType =
        i2c_poller::I2CPoller<freertos_message_queue::FreeRTOSMessageQueue>;
    using PressureTaskType = pressure_sensor_task::PressureSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue, I2CWriterType,
        I2CPollerType, CanClient>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        sensor_task_utils::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PressureTaskType, I2CWriterType,
                                    I2CPollerType, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer, I2CPollerType& poller,
               CanClient& can_client) -> PressureTaskType& {
        task.start(priority, "pressure", &writer, &poller, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    PressureTaskType task_entry;
    TaskType task;
};

}  // namespace pressure_sensor_task_starter
