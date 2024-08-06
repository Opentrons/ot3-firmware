#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/firmware/i2c_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace pipettes_tasks {

/**
 * Start pipettes tasks.
 */
void start_tasks(can::bus::CanBus& can_bus,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     motion_controller,
                 i2c::hardware::I2CBase& i2c3_interface,
                 i2c::hardware::I2CBase& i2c1_interface,
                 sensors::hardware::SensorHardwareBase& sensor_hardware,
                 spi::hardware::SpiDeviceBase& spi_device,
                 tmc2130::configs::TMC2130DriverConfig& driver_configs,
                 can::ids::NodeId id,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    void send_motor_driver_queue_isr(const tmc2130::tasks::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue(const sensors::utils::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        tmc2130_driver_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
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
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi_queue{nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    tmc2130::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* tmc2130_driver{nullptr};
    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};

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
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};

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
