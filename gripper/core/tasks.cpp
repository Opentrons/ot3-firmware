#include "gripper/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "gripper/core/can_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

using namespace gripper_tasks;

static auto main_tasks = MainTasks{};
static auto main_queues = MainQueueClient{};

/**
 * Start gripper tasks.
 */
void start_all_tasks(can_bus::CanBus& can_bus,
                     motor_class::Motor<lms::LeadScrewConfig>& z_motor,
                     brushed_motor::BrushedMotor& grip_motor) {
    // Start can task
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    main_tasks.can_writer = &can_writer;
    main_queues.set_queue(&can_writer.get_queue());

    // Start z motor tasks
    z_tasks::start_tasks(z_motor, &can_writer.get_queue());

    // Start g motor tasks
    g_tasks::start_tasks(grip_motor, &can_writer.get_queue());
}

MainQueueClient::MainQueueClient()
    : can_message_writer::MessageWriter{can_ids::NodeId::gripper} {}

/**
 * Access to the tasks singleton
 * @return
 */
auto get_main_tasks() -> MainTasks& { return main_tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto get_main_queues() -> MainQueueClient& { return main_queues; }
