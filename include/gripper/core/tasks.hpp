#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace gripper_tasks {

/**
 * Start gripper tasks.
 */
void start_tasks(can_bus::CanBus& can_bus,
                 motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                 brushed_motor::BrushedMotor& grip_motor,
                 spi::hardware::SpiDeviceBase& spi_device,
                 tmc2130::configs::TMC2130DriverConfig& driver_configs);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient(can_ids::NodeId this_fw);

    void send_brushed_motor_driver_queue(
        const brushed_motor_driver_task::TaskMessage& m);

    void send_brushed_motion_controller_queue(
        const brushed_motion_controller_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motor_driver_task::TaskMessage>* brushed_motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        brushed_motion_controller_task::TaskMessage>* brushed_motion_queue{
        nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    message_writer_task::MessageWriterTask<
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

namespace z_tasks {

void start_task(motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                spi::hardware::SpiDeviceBase& spi_device,
                tmc2130::configs::TMC2130DriverConfig& driver_configs,
                AllTask tasks);

struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

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
};

[[nodiscard]] auto get_queues() -> QueueClient&;

}  // namespace z_tasks

namespace g_tasks {

void start_task(brushed_motor::BrushedMotor& grip_motor);

}  // namespace g_tasks

}  // namespace gripper_tasks
