#pragma once

#include "can/core/ids.hpp"
#include "can/core/can_writer_task.hpp"
#include "can/core/message_writer.hpp"

#include "common/core/freertos_message_queue.hpp"

#include "eeprom/core/task.hpp"

#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"

#include "i2c/core/writer.hpp"
#include "i2c/core/poller.hpp"

namespace sensor_tasks {

using CanWriterTask = message_writer_task::MessageWriterTask<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CClient = i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CPollerClient = i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(
    CanWriterTask& can_writer,
    I2CClient& i2c3_task_client,
    I2CClient& i2c1_task_client,
    I2CPollerClient& i2c1_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware,
    can_ids::NodeId id);

/**
 * Access to all tasks pipette peripheral tasks. This will be a singleton.
 */
struct Tasks {
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
    sensors::tasks::EnvironmentSensorTask<
    freertos_message_queue::FreeRTOSMessageQueue>* environment_sensor_task{
        nullptr};
    sensors::tasks::CapacitiveSensorTask<
    freertos_message_queue::FreeRTOSMessageQueue>* capacitive_sensor_task{
        nullptr};
    sensors::tasks::PressureSensorTask<
    freertos_message_queue::FreeRTOSMessageQueue>* pressure_sensor_task{
        nullptr};
};

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue(const sensors::utils::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        environment_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        pressure_sensor_queue{nullptr};
};

/**
 * Access to the gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;


/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

} // namespace sensor_tasks