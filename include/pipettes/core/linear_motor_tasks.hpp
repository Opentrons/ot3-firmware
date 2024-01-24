#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "spi/core/writer.hpp"

/**
 * Linear Motor Tasks
 *
 * All tasks that deal with motion control for linear motion on pipettes.
 *
 * For the single/8 channel, the tmc2130 driver is used.
 * For the 96 channel, the tmc2160 driver is used.
 */
namespace linear_motor_tasks {

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using SPIWriterClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

// single channel/8 channel linear motor tasks
void start_tasks(
    CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    SPIWriterClient& spi_writer,
    tmc2130::configs::TMC2130DriverConfig& linear_driver_configs,
    can::ids::NodeId, motor_hardware_task::MotorHardwareTask& lmh_tsk,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>&
        tail_accessor);

// 96/384 linear motor tasks
void start_tasks(
    CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    SPIWriterClient& spi_writer,
    tmc2160::configs::TMC2160DriverConfig& linear_driver_configs,
    can::ids::NodeId, motor_hardware_task::MotorHardwareTask& lmh_tsk,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>&
        tail_accessor);

/**
 * Access to all the linear motion task queues on the pipette.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_usage_storage_queue(const usage_storage_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        usage_storage_task::TaskMessage>* usage_storage_queue{nullptr};
};

/**
 * Access to all the linear motion tasks on the pipette. This will be a
 * singleton.
 */
struct Tasks {
    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* usage_storage_task{
        nullptr};
    eeprom::data_rev_task::UpdateDataRevTask<
        freertos_message_queue::FreeRTOSMessageQueue>* update_data_rev_task{
        nullptr};
};

/**
 * Access to the linear tasks singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

/**
 * Access to the linear queues singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

namespace tmc2130_driver {

/**
 * Tasks related specifically to the the tmc2130 driver.
 */
struct Tasks {
    tmc2130::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};
};

/**
 * Queues related specifically to the the tmc2130 driver.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient()
        : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m) const {
        driver_queue->try_write(m);
    }

    void send_motor_driver_queue_isr(const tmc2130::tasks::TaskMessage& m) const {
        static_cast<void>(driver_queue->try_write_isr(m));
    }

    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        driver_queue{nullptr};
};

/**
 * Access to the tmc2130 queue.
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

/**
 * Access to the tmc2130 driver task.
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

}  // namespace tmc2130_driver

namespace tmc2160_driver {

/**
 * Tasks related specifically to the the tmc2160 driver.
 */
struct Tasks {
    tmc2160::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};
};

/**
 * Queues related specifically to the the tmc2160 driver.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient()
        : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

    void send_motor_driver_queue(const tmc2160::tasks::TaskMessage& m) const {
        driver_queue->try_write(m);
    }

    void send_motor_driver_queue_isr(const tmc2160::tasks::TaskMessage& m) const {
        static_cast<void>(driver_queue->try_write_isr(m));
    }

    freertos_message_queue::FreeRTOSMessageQueue<tmc2160::tasks::TaskMessage>*
        driver_queue{nullptr};
};

/**
 * Access to the tmc2160 queue.
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

/**
 * Access to the tmc2160 driver task.
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

}  // namespace tmc2160_driver

}  // namespace linear_motor_tasks
