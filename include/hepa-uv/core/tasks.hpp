#pragma once
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"
#include "hepa-uv/core/pushbutton_task.hpp"

namespace hepauv_tasks {

/**
 * Start gripper tasks.
 */
void start_tasks(can::bus::CanBus& can_bus,
                 i2c::hardware::I2CBase& i2c2, i2c::hardware::I2CBase& i2c3,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c2_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c3_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c2_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c3_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c2_task{nullptr};
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c2_poller_task{
        nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
   eeprom::data_rev_task::UpdateDataRevTask<
        freertos_message_queue::FreeRTOSMessageQueue>* update_data_rev_task{
        nullptr};

    // PushButtonBlink Test task
    pushbutton_task::PushButtonTask<freertos_message_queue::FreeRTOSMessageQueue>*
        pushbutton_task{nullptr};
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

[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace hepauv_tasks
