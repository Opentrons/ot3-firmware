#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class PressureSensorTaskStarter {
  public:
    using I2CWriterType =
        i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
    using I2CPollerType =
        i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;
    using PressureTaskType =
        PressureSensorTask<freertos_message_queue::FreeRTOSMessageQueue,
                           I2CWriterType, I2CPollerType, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<utils::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, PressureTaskType>;

    PressureSensorTaskStarter() : task_entry{queue}, task{task_entry} {}
    PressureSensorTaskStarter(const PressureSensorTaskStarter& c) = delete;
    PressureSensorTaskStarter(const PressureSensorTaskStarter&& c) = delete;
    auto operator=(const PressureSensorTaskStarter& c) = delete;
    auto operator=(const PressureSensorTaskStarter&& c) = delete;
    ~PressureSensorTaskStarter() = default;

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
}  // namespace tasks
}  // namespace sensors
