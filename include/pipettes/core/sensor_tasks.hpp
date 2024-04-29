#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "sensors/core/mmr920.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "sensors/core/tasks/tip_presence_notification_task.hpp"

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
namespace sensor_tasks {

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using I2CClient =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;
using I2CPollerClient =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(CanWriterTask& can_writer, I2CClient& i2c3_task_client,
                 I2CPollerClient& i2c3_poller_client,
                 I2CClient& i2c1_task_client,
                 I2CPollerClient& i2c1_poller_client,
                 sensors::hardware::SensorHardwareBase& sensor_hardware_primary,
                 can::ids::NodeId id,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware,
                 sensors::mmr920::SensorVersion sensor_version);

void start_tasks(
    CanWriterTask& can_writer, I2CClient& i2c3_task_client,
    I2CPollerClient& i2c3_poller_client, I2CClient& i2c1_task_client,
    I2CPollerClient& i2c1_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware_primary,
    sensors::hardware::SensorHardwareBase& sensor_hardware_secondary,
    can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware,
    sensors::mmr920::SensorVersion sensor_version);

/**
 * Access to all sensor/eeprom tasks. This will be a singleton.
 */
struct Tasks {
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
    sensors::tasks::EnvironmentSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>* environment_sensor_task{
        nullptr};
    sensors::tasks::CapacitiveSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        capacitive_sensor_task_front{nullptr};
    sensors::tasks::CapacitiveSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        capacitive_sensor_task_rear{nullptr};
    sensors::tasks::PressureSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        pressure_sensor_task_rear{nullptr};
    sensors::tasks::PressureSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        pressure_sensor_task_front{nullptr};
    sensors::tasks::TipPresenceNotificationTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        tip_notification_task_rear{nullptr};
    sensors::tasks::TipPresenceNotificationTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        tip_notification_task_front{nullptr};
};

/**
 * Access to all sensor/eeprom task queues.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue_rear(
        const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue_front(
        const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue_rear_isr(
        const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue_front_isr(
        const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue_rear(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue_front(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue_rear_isr(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue_front_isr(const sensors::utils::TaskMessage& m);

    void send_tip_notification_queue_rear(
        const sensors::tip_presence::TaskMessage& m);

    void send_tip_notification_queue_front(
        const sensors::tip_presence::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        environment_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue_rear{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue_front{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        pressure_sensor_queue_rear{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        pressure_sensor_queue_front{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        sensors::tip_presence::TaskMessage>* tip_notification_queue_rear{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        sensors::tip_presence::TaskMessage>* tip_notification_queue_front{
        nullptr};
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
