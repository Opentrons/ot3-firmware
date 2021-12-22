#include "pipettes/core/eeprom_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

using namespace pipettes_eeprom_task;

static auto queue =
    freertos_message_queue::FreeRTOSMessageQueue<eeprom_task::TaskMessage>{};

static auto task_entry = EEPromTaskType{queue};
static auto task =
    freertos_task::FreeRTOSTask<512, EEPromTaskType, i2c::I2C,
                                pipettes_tasks::QueueClient>{task_entry};

auto pipettes_eeprom_task::start_task(i2c::I2C &driver,
                                      pipettes_tasks::QueueClient &client)
    -> EEPromTaskType & {
    task.start(5, "eeprom task", &driver, &client);
    return task_entry;
}