#include "FreeRTOS.h"
#include "task.h"
#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_message_queue_poller.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

static auto motor_driver_message_queue =
    freertos_message_queue::FreeRTOSMessageQueue<
        motor_driver_task::TaskMessage>{};
static auto motor_driver_task_handler =
    motor_driver_task::MotorDriverMessageHandler(
        interfaces::get_motor().driver);
static auto motor_driver_task_entry = motor_driver_task::MotorDriverTask(
    motor_driver_message_queue, motor_driver_task_handler);

static auto motion_controller_queue =
    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>{};
static auto motion_controller_task_handler =
    motion_controller_task::MotionControllerMessageHandler{
        interfaces::get_motor().motion_controller};
static auto motion_controller_task_entry =
    motion_controller_task::MotorControllerTask<lms::BeltConfig>{
        motion_controller_queue, motion_controller_task_handler};

static auto move_group_manager = move_group_task::MoveGroupType{};

static auto move_group_message_queue =
    freertos_message_queue::FreeRTOSMessageQueue<
        move_group_task::TaskMessage>{};
static auto move_group_task_handler =
    move_group_task::MoveGroupMessageHandler(move_group_manager);
static auto move_group_task_entry = move_group_task::MoveGroupTask(
    move_group_message_queue, move_group_task_handler);

int main() {
    interfaces::initialize();
    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    auto can_reader = can_task::start_reader(interfaces::get_can_bus());

    vTaskStartScheduler();
}
