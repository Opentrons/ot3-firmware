#include "pipettes/core/linear_motor_tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/firmware/eeprom_keys.hpp"

static auto motion_tasks = linear_motor_tasks::Tasks{};
static auto motion_queue_client = linear_motor_tasks::QueueClient{};

static auto tmc2130_tasks = linear_motor_tasks::tmc2130_driver::Tasks{};
static auto tmc2130_queue_client =
    linear_motor_tasks::tmc2130_driver::QueueClient{};

static auto tmc2160_tasks = linear_motor_tasks::tmc2160_driver::Tasks{};
static auto tmc2160_queue_client =
    linear_motor_tasks::tmc2160_driver::QueueClient{};

static auto mc_task_builder =
    freertos_task::TaskStarter<256,
                               motion_controller_task::MotionControllerTask>{};
static auto tmc2130_driver_task_builder =
    freertos_task::TaskStarter<256, tmc2130::tasks::MotorDriverTask>{};
static auto tmc2160_driver_task_builder =
    freertos_task::TaskStarter<256, tmc2160::tasks::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<256, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    256, move_status_reporter_task::MoveStatusReporterTask>{};
static auto linear_usage_storage_task_builder =
    freertos_task::TaskStarter<256, usage_storage_task::UsageStorageTask>{};
static auto eeprom_data_rev_update_builder =
    freertos_task::TaskStarter<256, eeprom::data_rev_task::UpdateDataRevTask>{};

auto linear_motor_tasks::start_tasks(
    linear_motor_tasks::CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>& motion_controller,
    linear_motor_tasks::SPIWriterClient& spi_writer,
    tmc2130::configs::TMC2130DriverConfig& linear_driver_configs,
    can::ids::NodeId id, motor_hardware_task::MotorHardwareTask& lmh_tsk,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>& tail_accessor) -> interfaces::linear_motor::diag0_handler {
    tmc2130_queue_client.set_node_id(id);
    motion_queue_client.set_node_id(id);

    auto& queues = linear_motor_tasks::get_queues();
    auto& tmc2130_queues = linear_motor_tasks::tmc2130_driver::get_queues();
    auto& tmc2130_tasks = linear_motor_tasks::tmc2130_driver::get_tasks();
    auto& motion_tasks = linear_motor_tasks::get_tasks();

    // Linear Motor Tasks
    auto& motion =
        mc_task_builder.start(5, "motion controller", motion_controller, queues,
                              queues, tmc2130_queues, queues);
    auto& tmc2130_driver = tmc2130_driver_task_builder.start(
        5, "tmc2130 driver", linear_driver_configs, queues, spi_writer);
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues, motion_controller.get_mechanical_config(),
        queues);
    auto& usage_storage_task = linear_usage_storage_task_builder.start(
        5, "usage storage", queues, sensor_tasks::get_queues(), tail_accessor);

    auto& eeprom_data_rev_update_task = eeprom_data_rev_update_builder.start(
        5, "data_rev_update", sensor_tasks::get_queues(), tail_accessor,
        table_updater);

    tmc2130_tasks.driver = &tmc2130_driver;
    motion_tasks.move_group = &move_group;
    motion_tasks.move_status_reporter = &move_status_reporter;
    motion_tasks.usage_storage_task = &usage_storage_task;
    motion_tasks.update_data_rev_task = &eeprom_data_rev_update_task;

    queues.set_queue(&can_writer.get_queue());
    tmc2130_queues.set_queue(&can_writer.get_queue());
    tmc2130_queues.driver_queue = &tmc2130_driver.get_queue();

    queues.motion_queue = &motion.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.move_status_report_queue = &move_status_reporter.get_queue();
    queues.usage_storage_queue = &usage_storage_task.get_queue();
    lmh_tsk.start_task();

    return linear_motor_tasks::call_run_diag0_interrupt;
}

auto linear_motor_tasks::start_tasks(
    linear_motor_tasks::CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>& motion_controller,
    linear_motor_tasks::SPIWriterClient& spi_writer,
    tmc2160::configs::TMC2160DriverConfig& linear_driver_configs,
    can::ids::NodeId id, motor_hardware_task::MotorHardwareTask& lmh_tsk,
    eeprom::dev_data::DevDataTailAccessor<sensor_tasks::QueueClient>& tail_accessor) -> interfaces::linear_motor::diag0_handler {
    tmc2160_queue_client.set_node_id(id);
    motion_queue_client.set_node_id(id);

    auto& queues = linear_motor_tasks::get_queues();
    auto& tmc2160_queues = linear_motor_tasks::tmc2160_driver::get_queues();
    auto& tmc2160_tasks = linear_motor_tasks::tmc2160_driver::get_tasks();
    auto& motion_tasks = linear_motor_tasks::get_tasks();

    // Linear Motor Tasks
    auto& motion =
        mc_task_builder.start(5, "motion controller", motion_controller, queues,
                              queues, tmc2160_queues, queues);
    auto& tmc2160_driver = tmc2160_driver_task_builder.start(
        5, "tmc2160 driver", linear_driver_configs, queues, spi_writer);
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues, motion_controller.get_mechanical_config(),
        queues);
    auto& usage_storage_task = linear_usage_storage_task_builder.start(
        5, "linear usage storage", queues, sensor_tasks::get_queues(),
        tail_accessor);
    auto& eeprom_data_rev_update_task = eeprom_data_rev_update_builder.start(
        5, "data_rev_update", sensor_tasks::get_queues(), tail_accessor,
        table_updater);

    tmc2160_tasks.driver = &tmc2160_driver;
    motion_tasks.motion_controller = &motion;
    motion_tasks.move_group = &move_group;
    motion_tasks.move_status_reporter = &move_status_reporter;
    motion_tasks.usage_storage_task = &usage_storage_task;
    motion_tasks.update_data_rev_task = &eeprom_data_rev_update_task;

    queues.set_queue(&can_writer.get_queue());
    tmc2160_queues.set_queue(&can_writer.get_queue());
    tmc2160_queues.driver_queue = &tmc2160_driver.get_queue();

    queues.motion_queue = &motion.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.move_status_report_queue = &move_status_reporter.get_queue();
    queues.usage_storage_queue = &usage_storage_task.get_queue();

    lmh_tsk.start_task();

    return linear_motor_tasks::call_run_diag0_interrupt;
}

void linear_motor_tasks::call_run_diag0_interrupt() {
    if (linear_motor_tasks::get_tasks().motion_controller) {
        return linear_motor_tasks::get_tasks().motion_controller->run_diag0_interrupt();
    }
}

linear_motor_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

void linear_motor_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void linear_motor_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void linear_motor_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void linear_motor_tasks::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
    usage_storage_queue->try_write(m);
}

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::get_queues() -> QueueClient& {
    return motion_queue_client;
}

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::get_tasks() -> Tasks& { return motion_tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::tmc2130_driver::get_tasks() -> Tasks& {
    return tmc2130_tasks;
}

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::tmc2160_driver::get_tasks() -> Tasks& {
    return tmc2160_tasks;
}

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::tmc2130_driver::get_queues() -> QueueClient& {
    return tmc2130_queue_client;
}

/**
 * Access to the queues singleton
 * @return
 */
auto linear_motor_tasks::tmc2160_driver::get_queues() -> QueueClient& {
    return tmc2160_queue_client;
}
