#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/brushed_move_group_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace gripper_tasks {

/**
 * Start gripper tasks.
 */
auto start_tasks(can::bus::CanBus& can_bus,
                 motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                 brushed_motor::BrushedMotor<lms::GearBoxConfig>& grip_motor,
                 spi::hardware::SpiDeviceBase& spi_device,
                 tmc2130::configs::TMC2130DriverConfig& driver_configs,
                 i2c::hardware::I2CBase& i2c2, i2c::hardware::I2CBase& i2c3,
                 sensors::hardware::SensorHardwareBase& sensor_hardware,
                 eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface,
                 motor_hardware_task::MotorHardwareTask& zmh_tsk,
                 motor_hardware_task::MotorHardwareTask& gmh_tsk)
                 -> z_motor_iface::diag0_handler;

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);

    void send_brushed_motor_driver_queue(
        const brushed_motor_driver_task::TaskMessage& m);

    void send_brushed_motion_controller_queue(
        const brushed_motion_controller_task::TaskMessage& m);

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    void send_capacitive_sensor_queue_front(
        const sensors::utils::TaskMessage& m);

    void send_capacitive_sensor_queue_rear(
        const sensors::utils::TaskMessage& m);

    void send_environment_sensor_queue(const sensors::utils::TaskMessage& m);

    void send_pressure_sensor_queue_front(const sensors::utils::TaskMessage& m);
    void send_pressure_sensor_queue_rear(const sensors::utils::TaskMessage& m);

    void send_tip_notification_queue_rear(
        const sensors::tip_presence::TaskMessage& m);
    void send_tip_notification_queue_front(
        const sensors::tip_presence::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motor_driver_task::TaskMessage>* brushed_motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motion_controller_task::TaskMessage>* brushed_motion_queue{
        nullptr};
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
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue_front{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<sensors::utils::TaskMessage>*
        capacitive_sensor_queue_rear{nullptr};
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
    brushed_motor_driver_task::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* brushed_motor_driver{
        nullptr};
    brushed_motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        brushed_motion_controller{nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
    brushed_move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* brushed_move_group{
        nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        brushed_move_status_reporter{nullptr};
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
    sensors::tasks::CapacitiveSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        capacitive_sensor_task_front{nullptr};
    sensors::tasks::CapacitiveSensorTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        capacitive_sensor_task_rear{nullptr};
    eeprom::data_rev_task::UpdateDataRevTask<
        freertos_message_queue::FreeRTOSMessageQueue>* update_data_rev_task{
        nullptr};
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* jaw_usage_storage_task{
        nullptr};
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* z_usage_storage_task{
        nullptr};
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

namespace z_tasks {

auto start_task(
    motor_class::Motor<lms::LeadScrewConfig>& z_motor,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs, AllTask& tasks,
    gripper_tasks::QueueClient& main_queues,
    eeprom::dev_data::DevDataTailAccessor<gripper_tasks::QueueClient>&
        tail_accessor) -> z_motor_iface::diag0_handler;

void call_run_diag0_interrupt();

struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_usage_storage_queue(const usage_storage_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        tmc2130_driver_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        usage_storage_task::TaskMessage>* z_usage_storage_queue{nullptr};
};

[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace z_tasks

namespace g_tasks {

void start_task(
    brushed_motor::BrushedMotor<lms::GearBoxConfig>& grip_motor,
    AllTask& gripper_tasks, gripper_tasks::QueueClient& main_queues,
    eeprom::dev_data::DevDataTailAccessor<gripper_tasks::QueueClient>&
        tail_accessor);

struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_brushed_motor_driver_queue(
        const brushed_motor_driver_task::TaskMessage& m);

    void send_brushed_motion_controller_queue(
        const brushed_motion_controller_task::TaskMessage& m);

    void send_brushed_move_group_queue(
        const brushed_move_group_task::TaskMessage& m);

    void send_brushed_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_usage_storage_queue(const usage_storage_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motor_driver_task::TaskMessage>* brushed_motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motion_controller_task::TaskMessage>* brushed_motion_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_move_group_task::TaskMessage>* brushed_move_group_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>*
        brushed_move_status_report_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        usage_storage_task::TaskMessage>* g_usage_storage_queue{nullptr};
};

[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace g_tasks

}  // namespace gripper_tasks
