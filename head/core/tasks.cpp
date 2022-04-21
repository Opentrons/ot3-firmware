#include "head/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "head/core/adc.hpp"
#include "head/core/can_task.hpp"

#include "head/core/tasks/presence_sensing_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto head_tasks_col = head_tasks::HeadTasks{};
static auto head_queues = head_tasks::HeadQueueClient{};

static auto spi2_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto spi3_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto left_tasks = head_tasks::MotorTasks{};
static auto left_queues = head_tasks::MotorQueueClient{can_ids::NodeId::head_l};
static auto right_tasks = head_tasks::MotorTasks{};
static auto right_queues =
    head_tasks::MotorQueueClient{can_ids::NodeId::head_r};

static auto left_mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};

static auto right_mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};

static auto left_motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};
static auto right_motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};

static auto left_move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto right_move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto left_move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};
static auto right_move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

static auto presence_sensing_driver_task_builder = freertos_task::TaskStarter<
    512, presence_sensing_driver_task::PresenceSensingDriverTask>{};

static auto spi2_task_builder = freertos_task::TaskStarter<512, spi::tasks::SPITask>{};
static auto spi3_task_builder = freertos_task::TaskStarter<512, spi::tasks::SPITask>{};

/**
 * Start head tasks.
 */
void head_tasks::start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        left_motion_controller,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        right_motion_controller,
    presence_sensing_driver::PresenceSensingDriver& presence_sensing_driver,
    spi::hardware::SpiDeviceBase& spi2_device,
    spi::hardware::SpiDeviceBase& spi3_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs) {
    // Start the head tasks
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    auto& spi2_task = spi2_task_builder.start(5, spi2_device);
    spi2_task_client.set_queue(&spi2_task.get_queue());
    auto& spi3_task = spi3_task_builder.start(5, spi3_device);
    spi3_task_client.set_queue(&spi3_task.get_queue());

    auto& presence_sensing = presence_sensing_driver_task_builder.start(
        5, "presence", presence_sensing_driver, head_queues);

    // Assign head task collection task pointers
    head_tasks_col.can_writer = &can_writer;
    head_tasks_col.presence_sensing_driver_task = &presence_sensing;

    // Assign head queue client message queue pointers
    head_queues.set_queue(&can_writer.get_queue());
    head_queues.presence_sensing_driver_queue = &presence_sensing.get_queue();

    // Start the left motor tasks
    auto& left_motion = left_mc_task_builder.start(
        5, "left mc", left_motion_controller, left_queues);
    auto& left_tmc2130_driver = left_motor_driver_task_builder.start(
        5, "left motor driver", driver_configs, left_queues, spi3_task_client);
    auto& left_motor = left_motor_driver_task_builder.start(
        5, "left motor driver", left_motor_driver, left_queues);
    auto& left_move_group = left_move_group_task_builder.start(
        5, "left move group", left_queues, left_queues);
    auto& left_move_status_reporter = left_move_status_task_builder.start(
        5, "left move status", left_queues,
        left_motion_controller.get_mechanical_config());

    // Assign left motor task collection task pointers
    left_tasks.motion_controller = &left_motion;
    left_tasks.tmc2130_driver = &left_tmc2130_driver;
    left_tasks.move_group = &left_move_group;
    left_tasks.move_status_reporter = &left_move_status_reporter;
    left_tasks.spi_task = &spi3_task;

    // Assign left motor queue client message queue pointers
    left_queues.motion_queue = &left_motion.get_queue();
    left_queues.motor_queue = &left_tmc2130_driver.get_queue();
    left_queues.move_group_queue = &left_move_group.get_queue();
    left_queues.set_queue(&can_writer.get_queue());
    left_queues.move_status_report_queue =
        &left_move_status_reporter.get_queue();

    // Start the right motor tasks
    auto& right_motion = right_mc_task_builder.start(
        5, "right mc", right_motion_controller, right_queues);
    auto& right_tmc2130_driver = right_motor_driver_task_builder.start(
        5, "right motor driver", driver_configs, right_queues, spi2_task_client);
    auto& right_move_group = right_move_group_task_builder.start(
        5, "right move group", right_queues, right_queues);
    auto& right_move_status_reporter = right_move_status_task_builder.start(
        5, "right move status", right_queues,
        right_motion_controller.get_mechanical_config());

    // Assign right motor task collection task pointers
    right_tasks.motion_controller = &right_motion;
    right_tasks.tmc2130_driver = &right_tmc2130_driver;
    right_tasks.move_group = &right_move_group;
    right_tasks.move_status_reporter = &right_move_status_reporter;
    right_tasks.spi_task = &spi2_task;

    // Assign right motor queue client message queue pointers
    right_queues.motion_queue = &right_motion.get_queue();
    right_queues.motor_queue = &right_tmc2130_driver.get_queue();
    right_queues.move_group_queue = &right_move_group.get_queue();
    right_queues.set_queue(&can_writer.get_queue());
    right_queues.move_status_report_queue =
        &right_move_status_reporter.get_queue();
}

// Implementation of HeadQueueClient

head_tasks::HeadQueueClient::HeadQueueClient()
    : can_message_writer::MessageWriter{can_ids::NodeId::head} {}

void head_tasks::HeadQueueClient::send_presence_sensing_driver_queue(
    const presence_sensing_driver_task::TaskMessage& m) {
    presence_sensing_driver_queue->try_write(m);
}

// Implementation of MotorQueueClient

head_tasks::MotorQueueClient::MotorQueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

void head_tasks::MotorQueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void head_tasks::MotorQueueClient::send_motor_driver_queue(
    const tmc2130::tasks::TaskMessage& m) {
    motor_queue->try_write(m);
}

void head_tasks::MotorQueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void head_tasks::MotorQueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

auto head_tasks::get_tasks() -> HeadTasks& { return head_tasks_col; }

auto head_tasks::get_queue_client() -> HeadQueueClient& { return head_queues; }

auto head_tasks::get_right_tasks() -> MotorTasks& { return right_tasks; }

auto head_tasks::get_left_tasks() -> MotorTasks& { return left_tasks; }

auto head_tasks::get_right_queues() -> MotorQueueClient& {
    return right_queues;
}

auto head_tasks::get_left_queues() -> MotorQueueClient& { return left_queues; }
