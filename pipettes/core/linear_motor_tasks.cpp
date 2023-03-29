#include "pipettes/core/linear_motor_tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "pipettes/core/sensor_tasks.hpp"

static auto motion_tasks = linear_motor_tasks::Tasks{};
static auto motion_queue_client = linear_motor_tasks::QueueClient{};

static auto tmc2130_tasks = linear_motor_tasks::tmc2130_driver::Tasks{};
static auto tmc2130_queue_client =
    linear_motor_tasks::tmc2130_driver::QueueClient{};

static auto tmc2160_tasks = linear_motor_tasks::tmc2160_driver::Tasks{};
static auto tmc2160_queue_client =
    linear_motor_tasks::tmc2160_driver::QueueClient{};

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto tmc2130_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};
static auto tmc2160_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2160::tasks::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

void linear_motor_tasks::start_tasks(
    linear_motor_tasks::CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    linear_motor_tasks::SPIWriterClient& spi_writer,
    tmc2130::configs::TMC2130DriverConfig& linear_driver_configs,
    can::ids::NodeId id, motor_hardware_task::MotorHardwareTask& lmh_tsk) {
    tmc2130_queue_client.set_node_id(id);
    motion_queue_client.set_node_id(id);

    auto& queues = linear_motor_tasks::get_queues();
    auto& tmc2130_queues = linear_motor_tasks::tmc2130_driver::get_queues();
    auto& tmc2130_tasks = linear_motor_tasks::tmc2130_driver::get_tasks();
    auto& motion_tasks = linear_motor_tasks::get_tasks();

    // Linear Motor Tasks
    auto& motion =
        mc_task_builder.start(5, "motion controller", motion_controller, queues,
                              sensor_tasks::get_queues());
    auto& tmc2130_driver = tmc2130_driver_task_builder.start(
        5, "tmc2130 driver", linear_driver_configs, queues, spi_writer);
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues, motion_controller.get_mechanical_config(),
        sensor_tasks::get_queues());

    tmc2130_tasks.driver = &tmc2130_driver;
    motion_tasks.move_group = &move_group;
    motion_tasks.move_status_reporter = &move_status_reporter;

    queues.set_queue(&can_writer.get_queue());
    tmc2130_queues.set_queue(&can_writer.get_queue());
    tmc2130_queues.driver_queue = &tmc2130_driver.get_queue();

    queues.motion_queue = &motion.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.move_status_report_queue = &move_status_reporter.get_queue();
    lmh_tsk.start_task();
}

void linear_motor_tasks::start_tasks(
    linear_motor_tasks::CanWriterTask& can_writer,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    linear_motor_tasks::SPIWriterClient& spi_writer,
    tmc2160::configs::TMC2160DriverConfig& linear_driver_configs,
    can::ids::NodeId id, motor_hardware_task::MotorHardwareTask& lmh_tsk) {
    tmc2160_queue_client.set_node_id(id);
    motion_queue_client.set_node_id(id);

    auto& queues = linear_motor_tasks::get_queues();
    auto& tmc2160_queues = linear_motor_tasks::tmc2160_driver::get_queues();
    auto& tmc2160_tasks = linear_motor_tasks::tmc2160_driver::get_tasks();
    auto& motion_tasks = linear_motor_tasks::get_tasks();

    // Linear Motor Tasks
    auto& motion =
        mc_task_builder.start(5, "motion controller", motion_controller, queues,
                              sensor_tasks::get_queues());
    auto& tmc2160_driver = tmc2160_driver_task_builder.start(
        5, "tmc2160 driver", linear_driver_configs, queues, spi_writer);
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues, motion_controller.get_mechanical_config(),
        sensor_tasks::get_queues());

    tmc2160_tasks.driver = &tmc2160_driver;
    motion_tasks.motion_controller = &motion;
    motion_tasks.move_group = &move_group;
    motion_tasks.move_status_reporter = &move_status_reporter;

    queues.set_queue(&can_writer.get_queue());
    tmc2160_queues.set_queue(&can_writer.get_queue());
    tmc2160_queues.driver_queue = &tmc2160_driver.get_queue();

    queues.motion_queue = &motion.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.move_status_report_queue = &move_status_reporter.get_queue();

    lmh_tsk.start_task();
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
