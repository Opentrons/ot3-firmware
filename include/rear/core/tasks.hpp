#pragma once
#include "common/core/freertos_timer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"

namespace rear_tasks {

/**
 * Start gripper tasks.
 */
void start_tasks(i2c::hardware::I2CBase& i2c3,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient {
    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c3_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c3_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr}; 
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
};

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_all_tasks() -> AllTask&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_main_queues() -> QueueClient&;

}  // namespace rear_tasks
