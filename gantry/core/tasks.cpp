#include "gantry/core/tasks.hpp"

#include "gantry/core/utils.hpp"

gantry_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

void gantry_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gantry_tasks::QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void gantry_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gantry_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    move_status_report_queue->try_write(m);
}

static auto tasks = gantry_tasks::AllTask{};
static auto queue_client = gantry_tasks::QueueClient{utils::get_node_id()};

/**
 * Access to the tasks singleton
 * @return
 */
auto gantry_tasks::get_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gantry_tasks::get_queues() -> QueueClient& { return queue_client; }
