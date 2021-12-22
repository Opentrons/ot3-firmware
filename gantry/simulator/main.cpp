#include "FreeRTOS.h"
#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/tasks.hpp"
#include "motor-control/core/tasks/motion_controller_task_starter.hpp"
#include "motor-control/core/tasks/motor_driver_task_starter.hpp"
#include "motor-control/core/tasks/move_group_task_starter.hpp"
#include "motor-control/core/tasks/move_status_reporter_task_starter.hpp"
#include "task.h"

static auto mc_task_builder =
    motion_controller_task_starter::TaskStarter<lms::BeltConfig, 512,
                                                gantry_tasks::QueueClient>{};
static auto motor_driver_task_builder =
    motor_driver_task_starter::TaskStarter<512, gantry_tasks::QueueClient>{};
static auto move_group_task_builder =
    move_group_task_starter::TaskStarter<512, gantry_tasks::QueueClient,
                                         gantry_tasks::QueueClient>{};
static auto move_status_task_builder =
    move_status_reporter_task_starter::TaskStarter<512,
                                                   gantry_tasks::QueueClient>{};

int main() {
    interfaces::initialize();

    auto& queues = gantry_tasks::get_queues();
    auto& tasks = gantry_tasks::get_tasks();

    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    can_task::start_reader(interfaces::get_can_bus());
    auto motion = mc_task_builder.start(
        5, interfaces::get_motor().motion_controller, queues);
    auto motor = motor_driver_task_builder.start(
        5, interfaces::get_motor().driver, queues);
    auto move_group = move_group_task_builder.start(5, queues, queues);
    auto move_status_reporter = move_status_task_builder.start(5, queues);

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
