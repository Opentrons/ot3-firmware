#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "pipettes/core/tasks/motion_controller_task.hpp"
#include "pipettes/core/tasks/move_group_task.hpp"
#include "pipettes/core/tasks/gear_move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/writer.hpp"

/**
 * Gear Motor Tasks
 *
 * All tasks that deal with motion control related to the 96 channel
 * pick up/drop tip motors.
 *
 * Both motors are driven by a tmc2130 driver.
 */
namespace gear_motor_tasks {

using CanWriterTask = message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using SPIWriterClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(CanWriterTask& can_writer,
                 pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>&
                     motion_controller,
                 SPIWriterClient& spi_writer,
                 tmc2130::configs::TMC2130DriverConfig& gear_driver_configs,
                 can_ids::NodeId id);

/**
 * Access to all the gear motion tasks.
 */
struct Tasks {
    tmc2130::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};

    pipettes::tasks::motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};

    pipettes::tasks::gear_move_status::MoveStatusReporterTask<
    freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    pipettes::tasks::move_group_task::MoveGroupTask<
    freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
};

/**
 * Access to all the gear motion task queues.
 */
struct QueueClient : can_message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const pipettes::tasks::motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    void send_move_group_queue(const pipettes::tasks::move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const pipettes::tasks::gear_move_status::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::motion_controller_task::TaskMessage>* motion_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::move_group_task::TaskMessage>* move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        pipettes::tasks::gear_move_status::TaskMessage>* move_status_report_queue{
        nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        driver_queue{nullptr};
};

/**
 * Access to the gear motor tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> Tasks&;

/**
 * Access to the gear motor tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;

/**
 * Access to the left gear motor tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_right_gear_queues() -> MotorQueueClient&;

/**
 * Access to the right gear motor tasks queues singleton
 * @return
 */
[[nodiscard]] auto get_left_gear_queues() -> MotorQueueClient&;

}  // namespace gear_motor_tasks