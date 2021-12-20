#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace gantry_tasks {

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient(can_ids::NodeId this_fw);

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const motor_driver_task::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue;
    freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>* motor_queue;
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue;
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer;
    motor_driver_task::MotorDriverTask<QueueClient>* motor_driver;
    motion_controller_task::MotionControllerTask<lms::BeltConfig, QueueClient>*
        motion_controller;
    move_status_reporter_task::MoveStatusReporterTask<QueueClient>*
        move_status_reporter;
    move_group_task::MoveGroupTask<QueueClient, QueueClient>* move_group;
};

/**
 * Access to the tasks singleton
 * @return
 */
auto get_tasks() -> AllTask&;

/**
 * Access to the queues singleton
 * @return
 */
auto get_queues() -> QueueClient&;

}  // namespace gantry_tasks
