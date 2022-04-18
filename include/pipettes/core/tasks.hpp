#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/firmware/i2c_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"

namespace pipettes_tasks {

/**
 * Start pipettes tasks with one i2c bus.
 */
void start_tasks(can_bus::CanBus& can_bus,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     motion_controller,
                 motor_driver::MotorDriver& motor_driver,
                 i2c::hardware::I2CDeviceBase& i2c,
                 sensors::hardware::SensorHardwareBase& sensor_hardware,
                 can_ids::NodeId id);

/**
 * Start pipettes tasks with two i2c buses.
 */
void start_tasks(can_bus::CanBus& can_bus,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     motion_controller,
                 motor_driver::MotorDriver& motor_driver,
                 i2c::hardware::I2CDeviceBase& i2c3_device,
                 i2c::hardware::I2CDeviceBase& i2c1_device,
                 sensors::hardware::SensorHardwareBase& sensor_hardware,
                 can_ids::NodeId id);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const motor_driver_task::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_eeprom_queue(const eeprom_task::TaskMessage& m);

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue(const sensors::utils::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>* motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<eeprom_task::TaskMessage>*
        eeprom_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        environment_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        pressure_sensor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c3_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c1_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c3_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c1_poller_queue{nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    motor_driver_task::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue, QueueClient>*
        motor_driver{nullptr};
    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue, lms::LeadScrewConfig,
        QueueClient>* motion_controller{nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue, QueueClient,
        lms::LeadScrewConfig>* move_status_reporter{nullptr};
    move_group_task::MoveGroupTask<freertos_message_queue::FreeRTOSMessageQueue,
                                   QueueClient, QueueClient>* move_group{
        nullptr};

    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c1_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c1_poller_task{
        nullptr};
    eeprom_task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
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
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> AllTask&;

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace pipettes_tasks
