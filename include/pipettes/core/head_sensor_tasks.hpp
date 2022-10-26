#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"


/**
 * Sensor Tasks
 *
 * Handles all tasks that use the i2c clients.
 *
 * Capacitive sensor
 * Pressure sensor
 * Humidity sensor
 * Eeprom
 */
namespace head_sensor_tasks {

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using I2CClient =
i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CPollerClient =
i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(CanWriterTask& can_writer, I2CClient& i2c3_task_client,
                 I2CPollerClient& i2c3_poller_client,
                 can::ids::NodeId id,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware);

/**
 * Access to all sensor/eeprom tasks. This will be a singleton.
 */
struct Tasks {
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
};

/**
 * Access to all sensor/eeprom task queues.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr};
};

/**
 * Access to the sensor tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the sensor queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace sensor_tasks