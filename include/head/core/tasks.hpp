#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "head/core/tasks/presence_sensing_driver_task.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"

namespace head_tasks {

/**
 * Start head tasks.
 */

void start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        left_motion_controller,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        right_motion_controller,
    presence_sensing_driver::PresenceSensingDriver& presence_sensing_driver,
    spi::hardware::SpiDeviceBase& spi2_device,
    spi::hardware::SpiDeviceBase& spi3_device,
    tmc2130::configs::TMC2130DriverConfig& configs);

/**
 * The client for all head message queues not associated with a single motor.
 * This will be a singleton.
 */
struct HeadQueueClient : can_message_writer::MessageWriter {
    HeadQueueClient();

    void send_presence_sensing_driver_queue(
        const presence_sensing_driver_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        presence_sensing_driver_task::TaskMessage>*
        presence_sensing_driver_queue{nullptr};
};

/**
 * Access to all tasks not associated with a motor. This will be a singleton.
 */
struct HeadTasks {
    message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    presence_sensing_driver_task::PresenceSensingDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        presence_sensing_driver_task{nullptr};
};

/**
 * The client for all the per motor message queues. There will be one for the
 * left and one for the right.
 */
struct MotorQueueClient : can_message_writer::MessageWriter {
    MotorQueueClient(can_ids::NodeId this_fw);

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        motor_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi2_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi3_queue{nullptr};
};

/**
 * Access to all tasks associated with a motor. There will be one for the left
 * and one for the right.
 */
struct MotorTasks {
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
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
};

/**
 * Access to the head tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> HeadTasks&;

/**
 * Access to the head queues singleton
 * @return
 */
[[nodiscard]] auto get_queue_client() -> HeadQueueClient&;

/**
 * Access to the right tasks singleton
 * @return
 */
[[nodiscard]] auto get_right_tasks() -> MotorTasks&;

/**
 * Access to the left tasks singleton
 * @return
 */
[[nodiscard]] auto get_left_tasks() -> MotorTasks&;

/**
 * Access to the left queues singleton
 * @return
 */
[[nodiscard]] auto get_left_queues() -> MotorQueueClient&;

/**
 * Access to the right queues singleton
 * @return
 */
[[nodiscard]] auto get_right_queues() -> MotorQueueClient&;

}  // namespace head_tasks
