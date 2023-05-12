#pragma once
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_timer.hpp"
#include "common/core/message_queue.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/queues.hpp"
#include "rear-panel/core/tasks/hardware_task.hpp"
#include "rear-panel/core/tasks/host_comms_task.hpp"
#include "rear-panel/core/tasks/light_control_task.hpp"
#include "rear-panel/core/tasks/system_task.hpp"

namespace rear_panel_tasks {

template <typename RTOSHandle, class PortableTask>
struct Task {
    RTOSHandle handle;
    PortableTask* task;
};

/**
 * Start rear-panel tasks.
 */
void start_tasks(light_control_task::LightControlInterface& light_hardware,
                 i2c::hardware::I2CBase& i2c3,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface);

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
    host_comms_task::HostCommTask<freertos_message_queue::FreeRTOSMessageQueue>*
        host_comms_task{nullptr};

    light_control_task::LightControlTask<
        freertos_message_queue::FreeRTOSMessageQueue>* light_control_task{
        nullptr};

    system_task::SystemTask<freertos_message_queue::FreeRTOSMessageQueue>*
        system_task{nullptr};

    hardware_task::HardwareTask<freertos_message_queue::FreeRTOSMessageQueue>*
        hardware_task{nullptr};
};

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_all_tasks() -> AllTask&;

}  // namespace rear_panel_tasks
