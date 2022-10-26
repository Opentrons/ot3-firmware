#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "pipettes/core/tasks/gear_move_status_reporter_task.hpp"
#include "pipettes/core/tasks/motion_controller_task.hpp"
#include "pipettes/core/tasks/move_group_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"

namespace head_gear_tasks {

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using SPIWriterClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

/**
 * Start head tasks.
 */

void start_tasks(
    head_gear_tasks::CanWriterTask& can_writer, can::ids::NodeId id,
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>&
        left_motion_controller,
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>&
        right_motion_controller,
    SPIWriterClient& spi2_device, SPIWriterClient& spi3_device,
    tmc2160::configs::TMC2160DriverConfig& left_driver_configs,
    tmc2160::configs::TMC2160DriverConfig& right_driver_configs);

/**
 * Access to all tasks associated with a motor. There will be one for the left
 * and one for the right.
 */
struct Tasks {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    tmc2160::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};
    pipettes::tasks::motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};

    pipettes::tasks::gear_move_status::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    pipettes::tasks::move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
};

/**
 * Access to all the gear motion task queues.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const pipettes::tasks::motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2160::tasks::TaskMessage& m);

    void send_move_group_queue(
        const pipettes::tasks::move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const pipettes::tasks::gear_move_status::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::motion_controller_task::TaskMessage>* motion_queue{
        nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::move_group_task::TaskMessage>* move_group_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::gear_move_status::TaskMessage>*
        move_status_report_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<tmc2160::tasks::TaskMessage>*
        driver_queue{nullptr};
};

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

}  // namespace head_gear_tasks
