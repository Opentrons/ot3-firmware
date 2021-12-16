#include "FreeRTOS.h"
#include "task.h"
#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_message_queue_poller.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/task_holder.hpp"
#include "task.h"


int main() {
    interfaces::initialize();

    auto all_tasks = task_holder::TaskHolder();

    auto motor_driver_message_queue =
        freertos_message_queue::FreeRTOSMessageQueue<
            motor_driver_task::TaskMessage>{};
    auto motor_driver_task_handler =
        motor_driver_task::MotorDriverMessageHandler(
            interfaces::get_motor().driver);
    auto motor_driver_task_entry = motor_driver_task::MotorDriverTask(
        motor_driver_message_queue, motor_driver_task_handler);

    auto motion_controller_queue = freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>{};

    auto motion_controller_task_handler =
        motion_controller_task::MotionControllerMessageHandler{
            interfaces::get_motor().motion_controller};
    auto motion_controller_task_entry =
        motion_controller_task::MotorControllerTask<lms::BeltConfig>{
            motion_controller_queue, motion_controller_task_handler};

    all_tasks.motion_controller_queue = &motion_controller_queue;

    auto move_group_manager = move_group_task::MoveGroupType{};

    auto move_group_message_queue =
        freertos_message_queue::FreeRTOSMessageQueue<
            move_group_task::TaskMessage>{};
    auto move_group_task_handler =
        move_group_task::MoveGroupMessageHandler(move_group_manager, all_tasks);
    auto move_group_task_entry = move_group_task::MoveGroupTask(
        move_group_message_queue, move_group_task_handler);

    auto move_status_reporter_task_handler =
        move_status_reporter_task::MoveStatusMessageHandler();
    auto move_status_reporter_task_entry =
        move_status_reporter_task::MoveStatusReporterTask(
            interfaces::get_motor().completed_move_queue,
            move_status_reporter_task_handler);

    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    auto can_reader = can_task::start_reader(interfaces::get_can_bus());

    vTaskStartScheduler();
}
