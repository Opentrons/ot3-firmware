#include "gantry/core/tasks_rev1.hpp"

#include "gantry/core/can_task.hpp"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto tasks = gantry::tasks::AllTask{};
static auto queues = gantry::queues::QueueClient{utils::get_node_id()};

static auto spi_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2160::tasks::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

static auto spi_task_builder =
    freertos_task::TaskStarter<512, spi::tasks::Task>{};

/**
 * Start gantry ::tasks.
 */
void gantry::tasks::start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig, DefinedMotorHardware>& motion_controller,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2160::configs::TMC2160DriverConfig& driver_configs,
    motor_hardware_task::MotorHardwareTask<DefinedMotorHardware>& mh_tsk) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    auto& motion = mc_task_builder.start(5, "motion controller",
                                         motion_controller, ::queues);
    auto& tmc2160_driver = motor_driver_task_builder.start(
        5, "tmc2160 driver", driver_configs, ::queues, spi_task_client);
    auto& move_group =
        move_group_task_builder.start(5, "move group", ::queues, ::queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", ::queues, motion_controller.get_mechanical_config());

    auto& spi_task = spi_task_builder.start(5, "spi task", spi_device);
    spi_task_client.set_queue(&spi_task.get_queue());

    ::tasks.can_writer = &can_writer;
    ::tasks.motion_controller = &motion;
    ::tasks.tmc2160_driver = &tmc2160_driver;
    ::tasks.move_group = &move_group;
    ::tasks.move_status_reporter = &move_status_reporter;
    ::tasks.spi_task = &spi_task;

    ::queues.motion_queue = &motion.get_queue();
    ::queues.motor_driver_queue = &tmc2160_driver.get_queue();
    ::queues.move_group_queue = &move_group.get_queue();
    ::queues.set_queue(&can_writer.get_queue());
    ::queues.move_status_report_queue = &move_status_reporter.get_queue();
    ::queues.spi_queue = &spi_task.get_queue();

    mh_tsk.start_task();
}

gantry::queues::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void gantry::queues::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gantry::queues::QueueClient::send_motor_driver_queue(
    const tmc2160::tasks::TaskMessage& m) {
    motor_driver_queue->try_write(m);
}

void gantry::queues::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gantry::queues::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

/**
 * Access to the tasks singleton
 * @return
 */
auto gantry::tasks::get_tasks() -> AllTask& { return ::tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gantry::queues::get_queues() -> QueueClient& { return ::queues; }
