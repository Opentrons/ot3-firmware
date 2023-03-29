#include "pipettes/core/gear_motor_tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "pipettes/core/sensor_tasks.hpp"

// using namespace pipettes::tasks;

static auto left_gear_tasks = gear_motor_tasks::Tasks{};
static auto left_queue_client = gear_motor_tasks::QueueClient{};

static auto right_gear_tasks = gear_motor_tasks::Tasks{};
static auto right_queue_client = gear_motor_tasks::QueueClient{};

// left gear motor tasks
static auto mc_task_builder_left = freertos_task::TaskStarter<
    512, pipettes::tasks::motion_controller_task::MotionControllerTask>{};
static auto tmc2160_driver_task_builder_left =
    freertos_task::TaskStarter<512, tmc2160::tasks::gear::MotorDriverTask>{};

static auto move_group_task_builder_left = freertos_task::TaskStarter<
    512, pipettes::tasks::move_group_task::MoveGroupTask>{};
static auto move_status_task_builder_left = freertos_task::TaskStarter<
    512, pipettes::tasks::gear_move_status::MoveStatusReporterTask>{};

// right gear motor tasks
static auto mc_task_builder_right = freertos_task::TaskStarter<
    512, pipettes::tasks::motion_controller_task::MotionControllerTask>{};
static auto tmc2160_driver_task_builder_right =
    freertos_task::TaskStarter<512, tmc2160::tasks::gear::MotorDriverTask>{};

static auto move_group_task_builder_right = freertos_task::TaskStarter<
    512, pipettes::tasks::move_group_task::MoveGroupTask>{};
static auto move_status_task_builder_right = freertos_task::TaskStarter<
    512, pipettes::tasks::gear_move_status::MoveStatusReporterTask>{};

void gear_motor_tasks::start_tasks(
    gear_motor_tasks::CanWriterTask& can_writer,
    interfaces::gear_motor::GearMotionControl& motion_controllers,
    gear_motor_tasks::SPIWriterClient& spi_writer,
    motor_configs::HighThroughputPipetteDriverHardware& gear_driver_configs,
    can::ids::NodeId id,
    interfaces::gear_motor::GearMotorHardwareTasks& gmh_tsks) {
    left_queue_client.set_node_id(id);
    right_queue_client.set_node_id(id);

    auto& right_queues = gear_motor_tasks::get_right_gear_queues();
    auto& left_queues = gear_motor_tasks::get_left_gear_queues();
    auto& left_tasks = gear_motor_tasks::get_left_gear_tasks();
    auto& right_tasks = gear_motor_tasks::get_right_gear_tasks();

    // Left Gear Motor Tasks
    auto& motion_left = mc_task_builder_left.start(
        5, "motion controller", motion_controllers.left, left_queues,
        sensor_tasks::get_queues());
    auto& tmc2160_driver_left = tmc2160_driver_task_builder_left.start(
        5, "tmc2160 driver", gear_driver_configs.left_gear_motor, left_queues,
        spi_writer);
    auto& move_group_left = move_group_task_builder_left.start(
        5, "move group", left_queues, left_queues);
    auto& move_status_reporter_left = move_status_task_builder_left.start(
        5, "move status", left_queues,
        motion_controllers.left.get_mechanical_config(),
        sensor_tasks::get_queues());

    left_tasks.driver = &tmc2160_driver_left;
    left_tasks.motion_controller = &motion_left;
    left_tasks.move_group = &move_group_left;
    left_tasks.move_status_reporter = &move_status_reporter_left;

    left_queues.set_queue(&can_writer.get_queue());
    left_queues.driver_queue = &tmc2160_driver_left.get_queue();
    left_queues.motion_queue = &motion_left.get_queue();
    left_queues.move_group_queue = &move_group_left.get_queue();
    left_queues.move_status_report_queue =
        &move_status_reporter_left.get_queue();

    // Right Gear Motor Tasks
    auto& motion_right = mc_task_builder_right.start(
        5, "motion controller", motion_controllers.right, right_queues,
        sensor_tasks::get_queues());
    auto& tmc2160_driver_right = tmc2160_driver_task_builder_right.start(
        5, "tmc2160 driver", gear_driver_configs.right_gear_motor, right_queues,
        spi_writer);
    auto& move_group_right = move_group_task_builder_right.start(
        5, "move group", right_queues, right_queues);
    auto& move_status_reporter_right = move_status_task_builder_right.start(
        5, "move status", right_queues,
        motion_controllers.right.get_mechanical_config(),
        sensor_tasks::get_queues());

    right_tasks.driver = &tmc2160_driver_right;
    right_tasks.motion_controller = &motion_right;
    right_tasks.move_group = &move_group_right;
    right_tasks.move_status_reporter = &move_status_reporter_right;

    right_queues.set_queue(&can_writer.get_queue());
    right_queues.driver_queue = &tmc2160_driver_right.get_queue();
    right_queues.motion_queue = &motion_right.get_queue();
    right_queues.move_group_queue = &move_group_right.get_queue();
    right_queues.move_status_report_queue =
        &move_status_reporter_right.get_queue();

    gmh_tsks.left.start_task();
    gmh_tsks.right.start_task();
}

gear_motor_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

void gear_motor_tasks::QueueClient::send_motion_controller_queue(
    const pipettes::tasks::motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_motor_driver_queue(
    const tmc2160::tasks::gear::TaskMessage& m) {
    driver_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_move_group_queue(
    const pipettes::tasks::move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_move_status_reporter_queue(
    const pipettes::tasks::gear_move_status::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

auto gear_motor_tasks::get_right_gear_tasks() -> Tasks& {
    return right_gear_tasks;
}

auto gear_motor_tasks::get_right_gear_queues() -> QueueClient& {
    return right_queue_client;
}

auto gear_motor_tasks::get_left_gear_tasks() -> Tasks& {
    return left_gear_tasks;
}

auto gear_motor_tasks::get_left_gear_queues() -> QueueClient& {
    return left_queue_client;
}
