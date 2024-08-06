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
    256, pipettes::tasks::motion_controller_task::MotionControllerTask>{};
static auto tmc2160_driver_task_builder_left =
    freertos_task::TaskStarter<256, tmc2160::tasks::gear::MotorDriverTask>{};

static auto move_group_task_builder_left = freertos_task::TaskStarter<
    256, pipettes::tasks::move_group_task::MoveGroupTask>{};
static auto move_status_task_builder_left = freertos_task::TaskStarter<
    256, pipettes::tasks::gear_move_status::MoveStatusReporterTask>{};
static auto right_usage_storage_task_builder =
    freertos_task::TaskStarter<256, usage_storage_task::UsageStorageTask>{};

// right gear motor tasks
static auto mc_task_builder_right = freertos_task::TaskStarter<
    256, pipettes::tasks::motion_controller_task::MotionControllerTask>{};
static auto tmc2160_driver_task_builder_right =
    freertos_task::TaskStarter<256, tmc2160::tasks::gear::MotorDriverTask>{};

static auto move_group_task_builder_right = freertos_task::TaskStarter<
    256, pipettes::tasks::move_group_task::MoveGroupTask>{};
static auto move_status_task_builder_right = freertos_task::TaskStarter<
    256, pipettes::tasks::gear_move_status::MoveStatusReporterTask>{};
static auto left_usage_storage_task_builder =
    freertos_task::TaskStarter<256, usage_storage_task::UsageStorageTask>{};

void call_run_diag0_left_interrupt() {
    if (gear_motor_tasks::get_left_gear_tasks().motion_controller) {
        return gear_motor_tasks::get_left_gear_tasks()
            .motion_controller->run_diag0_interrupt();
    }
}

void call_run_diag0_right_interrupt() {
    if (gear_motor_tasks::get_right_gear_tasks().motion_controller) {
        return gear_motor_tasks::get_right_gear_tasks()
            .motion_controller->run_diag0_interrupt();
    }
}

auto gear_motor_tasks::start_tasks(
    gear_motor_tasks::CanWriterTask& can_writer,
    interfaces::gear_motor::GearMotionControl& motion_controllers,
    gear_motor_tasks::SPIWriterClient& spi_writer,
    motor_configs::HighThroughputPipetteDriverHardware& gear_driver_configs,
    can::ids::NodeId id,
    interfaces::gear_motor::GearMotorHardwareTasks& gmh_tsks,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>&
        tail_accessor)
    -> std::tuple<interfaces::diag0_handler, interfaces::diag0_handler> {
    left_queue_client.set_node_id(id);
    right_queue_client.set_node_id(id);

    auto& right_queues = gear_motor_tasks::get_right_gear_queues();
    auto& left_queues = gear_motor_tasks::get_left_gear_queues();
    auto& left_tasks = gear_motor_tasks::get_left_gear_tasks();
    auto& right_tasks = gear_motor_tasks::get_right_gear_tasks();

    // Left Gear Motor Tasks
    auto& motion_left = mc_task_builder_left.start(
        5, "motion controller", motion_controllers.left, left_queues,
        left_queues, left_queues, left_queues);
    auto& tmc2160_driver_left = tmc2160_driver_task_builder_left.start(
        5, "tmc2160 driver", gear_driver_configs.left_gear_motor, left_queues,
        spi_writer);
    auto& move_group_left = move_group_task_builder_left.start(
        5, "move group", left_queues, left_queues);
    auto& move_status_reporter_left = move_status_task_builder_left.start(
        5, "move status", left_queues,
        motion_controllers.left.get_mechanical_config(), left_queues);
    auto& left_usage_storage_task = left_usage_storage_task_builder.start(
        5, "usage storage", left_queues, sensor_tasks::get_queues(),
        tail_accessor);

    left_tasks.driver = &tmc2160_driver_left;
    left_tasks.motion_controller = &motion_left;
    left_tasks.move_group = &move_group_left;
    left_tasks.move_status_reporter = &move_status_reporter_left;
    left_tasks.usage_storage_task = &left_usage_storage_task;

    left_queues.set_queue(&can_writer.get_queue());
    left_queues.driver_queue = &tmc2160_driver_left.get_queue();
    left_queues.motion_queue = &motion_left.get_queue();
    left_queues.move_group_queue = &move_group_left.get_queue();
    left_queues.move_status_report_queue =
        &move_status_reporter_left.get_queue();
    left_queues.usage_storage_queue = &left_usage_storage_task.get_queue();

    // Right Gear Motor Tasks
    auto& motion_right = mc_task_builder_right.start(
        5, "motion controller", motion_controllers.right, right_queues,
        right_queues, right_queues, right_queues);
    auto& tmc2160_driver_right = tmc2160_driver_task_builder_right.start(
        5, "tmc2160 driver", gear_driver_configs.right_gear_motor, right_queues,
        spi_writer);
    auto& move_group_right = move_group_task_builder_right.start(
        5, "move group", right_queues, right_queues);
    auto& move_status_reporter_right = move_status_task_builder_right.start(
        5, "move status", right_queues,
        motion_controllers.right.get_mechanical_config(), right_queues);
    auto& right_usage_storage_task = right_usage_storage_task_builder.start(
        5, "usage storage", right_queues, sensor_tasks::get_queues(),
        tail_accessor);

    right_tasks.driver = &tmc2160_driver_right;
    right_tasks.motion_controller = &motion_right;
    right_tasks.move_group = &move_group_right;
    right_tasks.move_status_reporter = &move_status_reporter_right;
    right_tasks.usage_storage_task = &right_usage_storage_task;

    right_queues.set_queue(&can_writer.get_queue());
    right_queues.driver_queue = &tmc2160_driver_right.get_queue();
    right_queues.motion_queue = &motion_right.get_queue();
    right_queues.move_group_queue = &move_group_right.get_queue();
    right_queues.move_status_report_queue =
        &move_status_reporter_right.get_queue();
    right_queues.usage_storage_queue = &right_usage_storage_task.get_queue();

    gmh_tsks.left.start_task();
    gmh_tsks.right.start_task();

    return std::make_tuple(call_run_diag0_left_interrupt,
                           call_run_diag0_right_interrupt);
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

void gear_motor_tasks::QueueClient::send_motor_driver_queue_isr(
    const tmc2160::tasks::gear::TaskMessage& m) {
    static_cast<void>(driver_queue->try_write_isr(m));
}

void gear_motor_tasks::QueueClient::send_move_group_queue(
    const pipettes::tasks::move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_move_status_reporter_queue(
    const pipettes::tasks::gear_move_status::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void gear_motor_tasks::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
    usage_storage_queue->try_write(m);
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
