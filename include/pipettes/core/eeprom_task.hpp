#pragma once

#include "common/firmware/i2c_comms.hpp"
#include "pipettes/core/tasks.hpp"
#include "pipettes/core/tasks/eeprom.hpp"

namespace pipettes_eeprom_task {

using EEPromTaskType =
    eeprom_task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue,
                            i2c::I2C, pipettes_tasks::QueueClient>;

auto start_task(i2c::I2C& driver, pipettes_tasks::QueueClient& client)
    -> EEPromTaskType&;
}  // namespace pipettes_eeprom_task