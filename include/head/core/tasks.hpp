#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"

namespace head_tasks {

/**
 * Start head tasks.
 */
void start_tasks(can_bus::CanBus& can_bus,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     left_motion_controller,
                 motor_driver::MotorDriver& left_motor_driver,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     right_motion_controller,
                 motor_driver::MotorDriver& right_motor_driver,
                 presence_sensing_driver::PresenceSensingDriver& presence_Sensing_driver);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient(can_ids::NodeId this_fw);

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const motor_driver_task::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_presence_sensing_driver_queue(
        const send_presence_sensing_driver_task::TaskMessage& m);
    

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>* motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        presence_sensing_driver_task::TaskMessage>* presence_sensing_driver_task_queue{nullptr};
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
        freertos_message_queue::FreeRTOSMessageQueue, QueueClient>*
        move_status_reporter{nullptr};
    move_group_task::MoveGroupTask<freertos_message_queue::FreeRTOSMessageQueue,
                                   QueueClient, QueueClient>* move_group{
        nullptr};
    presence_sensing_driver_task::PresenceSensingDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue, QueueClient>* presence_sensing_driver_task{nullptr};
    

};

/**
 * Access to the right tasks singleton
 * @return
 */
[[nodiscard]] auto get_right_tasks() -> AllTask&;

/**
 * Access to the left tasks singleton
 * @return
 */
[[nodiscard]] auto get_left_tasks() -> AllTask&;

/**
 * Access to the left queues singleton
 * @return
 */
[[nodiscard]] auto get_left_queues() -> QueueClient&;

/**
 * Access to the right queues singleton
 * @return
 */
[[nodiscard]] auto get_right_queues() -> QueueClient&;

}  // namespace head_tasks
