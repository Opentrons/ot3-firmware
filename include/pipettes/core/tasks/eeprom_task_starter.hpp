#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

namespace eeprom_task_starter {

template <uint32_t StackDepth, eeprom::EEPromPolicy I2C,
          message_writer_task::TaskClient CanClient>
class TaskStarter {
  public:
    using EEPromTaskType =
        eeprom_task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue,
                                I2C, CanClient>;
    using QueueType =
        freertos_message_queue::FreeRTOSMessageQueue<eeprom_task::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, EEPromTaskType, I2C, CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    ~TaskStarter() = default;

    auto start(uint32_t priority, I2C& driver, CanClient& can_client)
        -> EEPromTaskType& {
        task.start(priority, "eeprom", &driver, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    EEPromTaskType task_entry;
    TaskType task;
};

}  // namespace eeprom_task_starter
