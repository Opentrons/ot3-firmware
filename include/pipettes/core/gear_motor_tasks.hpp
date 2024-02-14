#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/tasks/gear_tmc2160_motor_driver_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/core/tasks/gear_move_status_reporter_task.hpp"
#include "pipettes/core/tasks/motion_controller_task.hpp"
#include "pipettes/core/tasks/move_group_task.hpp"
#include "spi/core/writer.hpp"

/**
 * Gear Motor Tasks
 *
 * All tasks that deal with motion control related to the 96 channel
 * pick up/drop tip motors.
 *
 * Both motors are driven by a tmc2160 driver.
 */
namespace gear_motor_tasks {

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using SPIWriterClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

auto start_tasks(
    CanWriterTask& can_writer,
    interfaces::gear_motor::GearMotionControl& motion_controllers,
    SPIWriterClient& spi_writer,
    motor_configs::HighThroughputPipetteDriverHardware& gear_driver_configs,
    can::ids::NodeId id,
    interfaces::gear_motor::GearMotorHardwareTasks& gmh_tsks,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>&
        tail_accessor) -> std::tuple<interfaces::diag0_handler, interfaces::diag0_handler>;

void call_run_diag0_interrupt();

/**
 * Access to all the gear motion tasks.
 */
struct Tasks {
    tmc2160::tasks::gear::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};

    pipettes::tasks::motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};

    pipettes::tasks::gear_move_status::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    pipettes::tasks::move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* usage_storage_task{
        nullptr};
};

/**
 * Access to all the gear motion task queues.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const pipettes::tasks::motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2160::tasks::gear::TaskMessage& m);

    void send_motor_driver_queue_isr(
        const tmc2160::tasks::gear::TaskMessage& m);

    void send_move_group_queue(
        const pipettes::tasks::move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const pipettes::tasks::gear_move_status::TaskMessage& m);

    void send_usage_storage_queue(const usage_storage_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::motion_controller_task::TaskMessage>* motion_queue{
        nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::move_group_task::TaskMessage>* move_group_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::gear_move_status::TaskMessage>*
        move_status_report_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<
        tmc2160::tasks::gear::TaskMessage>* driver_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        usage_storage_task::TaskMessage>* usage_storage_queue{nullptr};
};

/**
 * Access to the right gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_right_gear_tasks() -> Tasks&;

/**
 * Access to the left gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_left_gear_tasks() -> Tasks&;

/**
 * Access to the left gear motor tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_right_gear_queues() -> QueueClient&;

/**
 * Access to the right gear motor tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_left_gear_queues() -> QueueClient&;

}  // namespace gear_motor_tasks
