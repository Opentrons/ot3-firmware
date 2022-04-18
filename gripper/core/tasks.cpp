#include "gripper/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "gripper/core/can_task.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

static auto tasks = gripper_tasks::AllTasks{can_ids::NodeId::gripper};

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, motor_driver_task::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};
static auto brushed_motor_driver_task_builder =
    freertos_task::TaskStarter<512,
                               brushed_motor_driver_task::MotorDriverTask>{};
static auto brushed_motion_controller_task_builder = freertos_task::TaskStarter<
    512, brushed_motion_controller_task::MotionControllerTask>{};
/**
 * Start gripper tasks.
 */
void gripper_tasks::start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    motor_driver::MotorDriver& motor_driver,
    brushed_motor_driver::BrushedMotorDriverIface& brushed_motor_driver,
    motor_hardware::BrushedMotorHardwareIface& brushed_motion_controller) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    auto& motion = mc_task_builder.start(5, "z mc", motion_controller, tasks);
    auto& motor = motor_driver_task_builder.start(5, "motor driver",
                                                  motor_driver, tasks);
    auto& move_group =
        move_group_task_builder.start(5, "move group", tasks, tasks);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", tasks, motion_controller.get_mechanical_config());
    auto& brushed_motor = brushed_motor_driver_task_builder.start(
        5, "bdc driver", brushed_motor_driver, tasks);
    auto& brushed_motion = brushed_motion_controller_task_builder.start(
        5, "bdc controller", brushed_motion_controller, tasks);

    tasks.set(&can_writer);
    tasks.set(&motion);
    tasks.set(&motor);
    tasks.set(&move_group);
    tasks.set(&move_status_reporter);
    tasks.set(&brushed_motor);
    tasks.set(&brushed_motion);
}


/**
 * Access to the tasks singleton
 * @return
 */
auto gripper_tasks::get_tasks() -> AllTasks& { return tasks; }
