#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace gantry_tasks {

/**
 * Start gantry tasks.
 */
void start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig>& motion_controller,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs);

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);

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

}  // namespace gantry_tasks
