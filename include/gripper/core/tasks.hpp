#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "common/core/tasks.hpp"

namespace gripper_tasks {

/**
 * Start gripper tasks.
 */
void start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    motor_driver::MotorDriver& motor_driver,
    brushed_motor_driver::BrushedMotorDriverIface& brushed_motor_driver,
    motor_hardware::BrushedMotorHardwareIface& brushed_motion_controller);

/**
 * Access to all tasks in the system; also acts as a queue client.
 */
using AllTasks = TaskHolder<freertos_message_queue::FreeRTOSMessageQueue,
    motor_driver_task::MotorDriverTask,
    motion_controller_task::MotionControllerTask,
    move_status_reporter_task::MoveStatusReporterTask,
    move_group_task::MoveGroupTask,
    brushed_motor_driver_task::MotorDriverTask,
    brushed_motion_controller_task::MotionControllerTask>;

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> AllTasks&;

}  // namespace gripper_tasks
