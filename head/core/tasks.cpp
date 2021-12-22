#include "head/core/tasks.hpp"

#include "head/firmware/can_task.hpp"
#include "motor-control/core/tasks/motion_controller_task_starter.hpp"
#include "motor-control/core/tasks/motor_driver_task_starter.hpp"
#include "motor-control/core/tasks/move_group_task_starter.hpp"
#include "motor-control/core/tasks/move_status_reporter_task_starter.hpp"

static auto left_tasks = head_tasks::AllTask{};
static auto left_queues = head_tasks::QueueClient{can_ids::NodeId::head_l};
static auto right_tasks = head_tasks::AllTask{};
static auto right_queues = head_tasks::QueueClient{can_ids::NodeId::head_r};

static auto left_mc_task_builder =
    motion_controller_task_starter::TaskStarter<lms::LeadScrewConfig, 512,
                                                head_tasks::QueueClient>{};
static auto right_mc_task_builder =
    motion_controller_task_starter::TaskStarter<lms::LeadScrewConfig, 512,
                                                head_tasks::QueueClient>{};
static auto left_motor_driver_task_builder =
    motor_driver_task_starter::TaskStarter<512, head_tasks::QueueClient>{};
static auto right_motor_driver_task_builder =
    motor_driver_task_starter::TaskStarter<512, head_tasks::QueueClient>{};
static auto left_move_group_task_builder =
    move_group_task_starter::TaskStarter<512, head_tasks::QueueClient,
                                         head_tasks::QueueClient>{};
static auto right_move_group_task_builder =
    move_group_task_starter::TaskStarter<512, head_tasks::QueueClient,
                                         head_tasks::QueueClient>{};
static auto left_move_status_task_builder =
    move_status_reporter_task_starter::TaskStarter<512,
                                                   head_tasks::QueueClient>{};
static auto right_move_status_task_builder =
    move_status_reporter_task_starter::TaskStarter<512,
                                                   head_tasks::QueueClient>{};

/**
 * Start gantry tasks.
 */
void head_tasks::start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        left_motion_controller,
    motor_driver::MotorDriver& left_motor_driver,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        right_motion_controller,
    motor_driver::MotorDriver& right_motor_driver) {
    // LEFT and RIGHT each get the same can task message queue
    auto can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    auto left_motion =
        left_mc_task_builder.start(5, left_motion_controller, left_queues);
    auto left_motor =
        left_motor_driver_task_builder.start(5, left_motor_driver, left_queues);
    auto left_move_group =
        left_move_group_task_builder.start(5, left_queues, left_queues);
    auto left_move_status_reporter =
        left_move_status_task_builder.start(5, left_queues);

    left_tasks.can_writer = &can_writer;
    left_tasks.motion_controller = &left_motion;
    left_tasks.motor_driver = &left_motor;
    left_tasks.move_group = &left_move_group;
    left_tasks.move_status_reporter = &left_move_status_reporter;

    left_queues.motion_queue = &left_motion.get_queue();
    left_queues.motor_queue = &left_motor.get_queue();
    left_queues.move_group_queue = &left_move_group.get_queue();
    left_queues.set_queue(&can_writer.get_queue());
    left_queues.move_status_report_queue =
        &left_move_status_reporter.get_queue();

    auto right_motion =
        right_mc_task_builder.start(5, right_motion_controller, right_queues);
    auto right_motor = right_motor_driver_task_builder.start(
        5, right_motor_driver, right_queues);
    auto right_move_group =
        right_move_group_task_builder.start(5, right_queues, right_queues);
    auto right_move_status_reporter =
        right_move_status_task_builder.start(5, right_queues);

    right_tasks.can_writer = &can_writer;
    right_tasks.motion_controller = &right_motion;
    right_tasks.motor_driver = &right_motor;
    right_tasks.move_group = &right_move_group;
    right_tasks.move_status_reporter = &right_move_status_reporter;

    right_queues.motion_queue = &right_motion.get_queue();
    right_queues.motor_queue = &right_motor.get_queue();
    right_queues.move_group_queue = &right_move_group.get_queue();
    right_queues.set_queue(&can_writer.get_queue());
    right_queues.move_status_report_queue =
        &right_move_status_reporter.get_queue();
}

head_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

void head_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void head_tasks::QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void head_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void head_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    move_status_report_queue->try_write(m);
}

auto head_tasks::get_right_tasks() -> AllTask& { return right_tasks; }

auto head_tasks::get_left_tasks() -> AllTask& { return left_tasks; }

auto head_tasks::get_right_queues() -> QueueClient& { return right_queues; }

auto head_tasks::get_left_queues() -> QueueClient& { return left_queues; }
