#include "FreeRTOS.h"
#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/motion_controller_task.hpp"
#include "gantry/core/motor_driver_task.hpp"
#include "gantry/core/move_group_task.hpp"
#include "gantry/core/move_status_reporter_task.hpp"
#include "gantry/core/tasks.hpp"
#include "task.h"

int main() {
    interfaces::initialize();

    auto& queues = gantry_tasks::get_queues();
    auto& tasks = gantry_tasks::get_tasks();

    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    can_task::start_reader(interfaces::get_can_bus());
    auto motion = gantry_motion_controller_task::start_task(
        interfaces::get_motor().motion_controller, queues);
    auto motor = gantry_motor_driver_task::start_task(
        interfaces::get_motor().driver, queues);
    auto move_group = gantry_move_group_task::start_task(queues);
    auto move_status_reporter =
        gantry_move_status_reporter_task::start_task(queues);

    tasks.can_writer = &can_writer;
    tasks.motion_controller = &motion;
    tasks.motor_driver = &motor;
    tasks.move_group = &move_group;
    tasks.move_status_reporter = &move_status_reporter;

    queues.motion_queue = &motion.get_queue();
    queues.motor_queue = &motor.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.set_queue(&can_writer.get_queue());
    queues.move_status_report_queue = &move_status_reporter.get_queue();

    vTaskStartScheduler();
}
