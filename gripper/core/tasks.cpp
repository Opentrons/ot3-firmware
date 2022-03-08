#include "gripper/core/tasks.hpp"

#include "gripper/core/can_task.hpp"
#include "motor-control/core/tasks/motion_controller_task_starter.hpp"
#include "motor-control/core/tasks/motor_driver_task_starter.hpp"
#include "motor-control/core/tasks/move_group_task_starter.hpp"
#include "motor-control/core/tasks/move_status_reporter_task_starter.hpp"

static auto tasks = gripper_tasks::AllTask{};
static auto queues = gripper_tasks::QueueClient{can_ids::NodeId::gripper};

static auto mc_task_builder =
    motion_controller_task_starter::TaskStarter<lms::BeltConfig, 512,
                                                gripper_tasks::QueueClient>{};
static auto motor_driver_task_builder =
    motor_driver_task_starter::TaskStarter<512, gripper_tasks::QueueClient>{};
static auto move_group_task_builder =
    move_group_task_starter::TaskStarter<512, gripper_tasks::QueueClient,
                                         gripper_tasks::QueueClient>{};
static auto move_status_task_builder =
    move_status_reporter_task_starter::TaskStarter<
        512, gripper_tasks::QueueClient>{};

/**
 * Start gripper tasks.
 */
void gripper_tasks::start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig>& motion_controller,
    motor_driver::MotorDriver& motor_driver) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    auto& motion = mc_task_builder.start(5, motion_controller, queues);
    auto& motor = motor_driver_task_builder.start(5, motor_driver, queues);
    auto& move_group = move_group_task_builder.start(5, queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(5, queues);

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
}

gripper_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

void gripper_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

/**
 * Access to the tasks singleton
 * @return
 */
auto gripper_tasks::get_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gripper_tasks::get_queues() -> QueueClient& { return queues; }
