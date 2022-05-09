#include "pipettes/core/gear_motor_tasks.hpp"

#include "common/core/freertos_task.hpp"

static auto tasks = gear_motor_tasks::Tasks{};
static auto queue_client = gear_motor_tasks::QueueClient{};


static auto mc_task_builder =
    freertos_task::TaskStarter<512,
        motion_controller_task::MotionControllerTask>{};
static auto tmc2130_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};


void gear_motor_tasks::start_tasks(
    gear_motor_tasks::CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>& motion_controller,
    gear_motor_tasks::SPIWriterClient& spi_writer,
    tmc2130::configs::TMC2130DriverConfig& gear_driver_configs,
    can_ids::NodeId id) {

    queue_client.set_node_id(id);

    auto& queues = gear_motor_tasks::get_queues();
    auto& tasks = gear_motor_tasks::get_tasks();

    // Gear Motor Tasks
    auto& motion = mc_task_builder.start(5, "motion controller",
                                         motion_controller, queues);
    auto& tmc2130_driver = tmc2130_driver_task_builder.start(
        5, "tmc2130 driver", gear_driver_configs, queues, spi_writer);

    tasks.driver = &tmc2130_driver;
    tasks.motion_controller = &motion;

    queues.set_queue(&can_writer.get_queue());
    queues.driver_queue = &tmc2130_driver.get_queue();
    queues.motion_queue = &motion.get_queue();
}

gear_motor_tasks::QueueClient::QueueClient()
// This gets overridden in start_tasks, needs to be static here since this
// is free-store allocated
    : can_message_writer::MessageWriter{can_ids::NodeId::pipette_left} {}


void gear_motor_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_motor_driver_queue(
    const tmc2130::tasks::TaskMessage& m) {
    driver_queue->try_write(m);
}


/**
 * Access to the gear motor tasks singleton
 * @return
 */
auto gear_motor_tasks::get_tasks() -> Tasks& {
    return tasks;
}


/**
 * Access to the queues singleton
 * @return
 */
auto gear_motor_tasks::get_queues() -> QueueClient& {
    return queue_client;
}