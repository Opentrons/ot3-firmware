#include "pipettes/core/gear_motor_tasks.hpp"

#include "common/core/freertos_task.hpp"

//using namespace pipettes::tasks;

static auto tasks = gear_motor_tasks::Tasks{};
static auto queue_client = gear_motor_tasks::QueueClient{};

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
        pipettes::tasks::motion_controller_task::MotionControllerTask>{};
static auto tmc2130_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};

static auto move_group_task_builder =
    freertos_task::TaskStarter<512, pipettes::tasks::move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, pipettes::tasks::gear_move_status::MoveStatusReporterTask>{};

void gear_motor_tasks::start_tasks(
    gear_motor_tasks::CanWriterTask& can_writer,
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>&
        motion_controller,
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
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues, motion_controller.get_mechanical_config());

    tasks.driver = &tmc2130_driver;
    tasks.motion_controller = &motion;
    tasks.move_group = &move_group;
    tasks.move_status_reporter = &move_status_reporter;

    queues.set_queue(&can_writer.get_queue());
    queues.driver_queue = &tmc2130_driver.get_queue();
    queues.motion_queue = &motion.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.move_status_report_queue = &move_status_reporter.get_queue();
}

gear_motor_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can_message_writer::MessageWriter{can_ids::NodeId::pipette_left} {}

void gear_motor_tasks::QueueClient::send_motion_controller_queue(
    const pipettes::tasks::motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gear_motor_tasks::QueueClient::send_motor_driver_queue(
    const tmc2130::tasks::TaskMessage& m) {
    driver_queue->try_write(m);
}

void send_move_group_queue(const pipettes::tasks::move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void send_move_status_reporter_queue(
    const pipettes::tasks::gear_move_status::TaskMessage& m) {
    static_cast<void>(send_move_status_reporter_queue->try_write_isr(m));
}

auto gear_motor_tasks::get_tasks() -> Tasks& { return tasks; }

auto gear_motor_tasks::get_queues() -> QueueClient& { return queue_client; }