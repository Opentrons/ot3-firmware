#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
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

using CanWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;
using SPIWriterClient =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>;

void start_tasks(CanWriterTask& can_writer,
                 motion_controller::MotionController<lms::LeadScrewConfig>&
                     motion_controller,
                 SPIWriterClient& spi_writer,
                 tmc2130::configs::TMC2130DriverConfig& gear_driver_configs,
                 can::ids::NodeId id);

/**
 * Access to all the gear motion tasks.
 */
struct Tasks {
    tmc2130::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* driver{nullptr};

    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};
};

/**
 * Access to all the gear motion task queues.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient();

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc2130::tasks::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<tmc2130::tasks::TaskMessage>*
        driver_queue{nullptr};

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
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

}  // namespace gear_motor_tasks