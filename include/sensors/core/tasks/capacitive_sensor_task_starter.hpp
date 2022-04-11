#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using I2CWriterType =
        i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
    using I2CPollerType =
        i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;
    using CapacitiveTaskType =
        CapacitiveSensorTask<freertos_message_queue::FreeRTOSMessageQueue,
                             I2CWriterType, I2CPollerType, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<utils::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, CapacitiveTaskType,
                                    I2CWriterType, I2CPollerType,
                                    hardware::SensorHardwareBase, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer, I2CPollerType& poller,
               hardware::SensorHardwareBase& sensor_hardware,
               CanClient& can_client) -> CapacitiveTaskType& {
        task.start(priority, "capacitive", &writer, &poller, &sensor_hardware,
                   &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    CapacitiveTaskType task_entry;
    TaskType task;
};
};  // namespace tasks
};  // namespace sensors
