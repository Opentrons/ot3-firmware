#include "gripper/core/tasks.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"

using namespace gripper_tasks::g_tasks;

static auto g_tasks = MotorTasks{};
static auto g_queues = QueueClient{can_ids::NodeId::gripper_g};

static auto brushed_motor_driver_task_builder =
    freertos_task::TaskStarter<512,
                               brushed_motor_driver_task::MotorDriverTask>{};
static auto brushed_motion_controller_task_builder = freertos_task::TaskStarter<
    512, brushed_motion_controller_task::MotionControllerTask>{};

// Start G tasks
void start_tasks(brushed_motor::BrushedMotor& grip_motor) {
    auto& brushed_motor = brushed_motor_driver_task_builder.start(
        5, "bdc driver", grip_motor.driver, queues);
    auto& brushed_motion = brushed_motion_controller_task_builder.start(
        5, "bdc controller", grip_motor.motion_controller, queues);

    g_tasks.brushed_motor_driver = &brushed_motor;
    g_tasks.brushed_motion_controller = &brushed_motion;

    g_queues.brushed_motor_queue = &brushed_motor.get_queue();
    g_queues.brushed_motion_queue = &brushed_motion.get_queue();
}

void QueueClient::send_brushed_motor_driver_queue(
    const brushed_motor_driver_task::TaskMessage& m) {
    brushed_motor_queue->try_write(m);
}

void QueueClient::send_brushed_motion_controller_queue(
    const brushed_motion_controller_task::TaskMessage& m) {
    brushed_motion_queue->try_write(m);
}

auto get_tasks() -> MotorTasks& { return g_tasks; }

auto get_queues() -> QueueClient& { return g_queues; }