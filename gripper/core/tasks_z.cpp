#include "common/core/freertos_task.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_driver_task.hpp"

using namespace gripper_tasks::z_tasks;

static auto z_tasks = MotorTasks{};
static auto z_queues = QueueClient{};

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, motor_driver_task::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

// Start Z tasks
void start_tasks(motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                 gripper_tasks::QueueType* can_queue) {
    auto& motion =
        mc_task_builder.start(5, "z mc", z_motor.motion_controller, z_queues);
    auto& motor = motor_driver_task_builder.start(5, "z motor driver",
                                                  z_motor.driver, z_queues);
    auto& move_group =
        move_group_task_builder.start(5, "z move group", z_queues, z_queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", z_queues,
        z_motor.motion_controller.get_mechanical_config());

    z_tasks.motion_controller = &motion;
    z_tasks.motor_driver = &motor;
    z_tasks.move_group = &move_group;
    z_tasks.move_status_reporter = &move_status_reporter;

    z_queues.motion_queue = &motion.get_queue();
    z_queues.motor_queue = &motor.get_queue();
    z_queues.move_group_queue = &move_group.get_queue();
    z_queues.move_status_report_queue = &move_status_reporter.get_queue();
    z_queues.set_queue(can_queue);
}

QueueClient::QueueClient()
    : can_message_writer::MessageWriter{can_ids::NodeId::gripper_z} {}

void QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void QueueClient::send_move_group_queue(const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

auto get_z_tasks() -> MotorTasks& { return z_tasks; }

auto get_z_queues() -> QueueClient& { return z_queues; }
