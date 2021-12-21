#include "pipettes/core/tasks.hpp"


pipettes_tasks::QueueClient::QueueClient()
    : can_message_writer::MessageWriter{can_ids::NodeId::pipette} {}

void pipettes_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    move_status_report_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_eeprom_queue(const eeprom_task::TaskMessage & m) {
    eeprom_queue->try_write(m);
}

static auto tasks = pipettes_tasks::AllTask{};
static auto queue_client = pipettes_tasks::QueueClient{};

/**
 * Access to the tasks singleton
 * @return
 */
auto pipettes_tasks::get_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto pipettes_tasks::get_queues() -> QueueClient& { return queue_client; }
