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
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace gripper_tasks {

using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>;

struct MainTasks {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
};

struct MainQueueClient : can_message_writer::MessageWriter {
    MainQueueClient();
};

[[nodiscard]] auto get_main_tasks() -> MainTasks&;

[[nodiscard]] auto get_main_queues() -> MainQueueClient&;

/**
 * Start gripper tasks.
 */
void start_all_tasks(can_bus::CanBus& can_bus,
                     motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                     brushed_motor::BrushedMotor& grip_motor);

namespace z_tasks {

void start_tasks(motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                 gripper_tasks::QueueType* can_queue);

struct MotorTasks {
    motor_driver_task::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motor_driver{nullptr};
    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
};

/**
 * Access to all the message queues for the z motor.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const motor_driver_task::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>* motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
};

[[nodiscard]] auto get_z_tasks() -> MotorTasks&;

[[nodiscard]] auto get_z_queues() -> QueueClient&;

}  // namespace z_tasks

namespace g_tasks {

void start_tasks(brushed_motor::BrushedMotor& grip_motor,
                 gripper_tasks::QueueType* can_queue);

struct MotorTasks {
    brushed_motor_driver_task::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* brushed_motor_driver{
        nullptr};
    brushed_motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        brushed_motion_controller{nullptr};
};

/**
 * Access to all the message queues for the g motor.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

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

[[nodiscard]] auto get_z_tasks() -> MotorTasks&;

[[nodiscard]] auto get_g_queues() -> QueueClient&;

}  // namespace g_tasks

}  // namespace gripper_tasks
