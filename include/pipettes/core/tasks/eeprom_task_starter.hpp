#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

namespace eeprom_task_starter {

template <uint32_t StackDepth, message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using I2CWriterType =
        i2c_writer::I2CWriter<freertos_message_queue::FreeRTOSMessageQueue>;
    using EEPromTaskType =
        eeprom_task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue,
                                I2CWriterType, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<eeprom_task::TaskMessage>;
    using TaskType = freertos_task::FreeRTOSTask<StackDepth, EEPromTaskType,
                                                 I2CWriterType, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2CWriterType& writer, CanClient& can_client)
        -> EEPromTaskType& {
        task.start(priority, "eeprom", &writer, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    EEPromTaskType task_entry;
    TaskType task;
};

}  // namespace eeprom_task_starter