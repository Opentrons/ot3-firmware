#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "pipettes/core/tasks/eeprom.hpp"
#include "common/firmware/i2c_comms.hpp"

namespace pipettes_tasks {

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

    void send_eeprom_queue(const eeprom_task::TaskMessage & m);

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
        eeprom_task::TaskMessage>* eeprom_queue{
        nullptr};
};

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    motor_driver_task::MotorDriverTask<QueueClient>* motor_driver{nullptr};
    motion_controller_task::MotionControllerTask<lms::LeadScrewConfig, QueueClient>*
        motion_controller{nullptr};
    move_status_reporter_task::MoveStatusReporterTask<QueueClient>*
        move_status_reporter{nullptr};
    move_group_task::MoveGroupTask<QueueClient, QueueClient>* move_group{
        nullptr};
    eeprom_task::EEPromTask<i2c::I2C, QueueClient>* eeprom_task{nullptr};
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
