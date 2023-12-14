#include "common/core/freertos_task.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

using namespace gripper_tasks;

static auto z_queues = z_tasks::QueueClient{};

static auto spi_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto spi_task_builder =
    freertos_task::TaskStarter<1048, spi::tasks::Task>{};

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};

static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};

static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};

static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};
#if PCBA_PRIMARY_REVISION != 'b'
static auto z_usage_storage_task_builder =
    freertos_task::TaskStarter<512, usage_storage_task::UsageStorageTask>{};
#endif

auto z_tasks::start_task(
    motor_class::Motor<lms::LeadScrewConfig>& z_motor,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs,
    AllTask& gripper_tasks, gripper_tasks::QueueClient& main_queues,
    eeprom::dev_data::DevDataTailAccessor<gripper_tasks::QueueClient>&
        tail_accessor) -> z_motor_iface::diag0_handler {
    auto& motion =
        mc_task_builder.start(5, "z mc", z_motor.motion_controller, z_queues,
                              z_queues, z_queues, z_queues);
    auto& move_group =
        move_group_task_builder.start(5, "move group", z_queues, z_queues);
    auto& tmc2130_driver = motor_driver_task_builder.start(
        5, "tmc2130 driver", driver_configs, z_queues, spi_task_client);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", z_queues,
        z_motor.motion_controller.get_mechanical_config(), z_queues);
    auto& spi_task = spi_task_builder.start(5, "spi", spi_device);
#if PCBA_PRIMARY_REVISION != 'b'
    auto& z_usage_storage_task = z_usage_storage_task_builder.start(
        5, "z usage storage", z_queues, main_queues, tail_accessor);
#else
    std::ignore = main_queues;
    std::ignore = tail_accessor;
#endif

    gripper_tasks.motion_controller = &motion;
    gripper_tasks.tmc2130_driver = &tmc2130_driver;
    gripper_tasks.move_group = &move_group;
    gripper_tasks.move_status_reporter = &move_status_reporter;
    gripper_tasks.spi_task = &spi_task;
#if PCBA_PRIMARY_REVISION != 'b'
    gripper_tasks.z_usage_storage_task = &z_usage_storage_task;
#endif

    z_queues.motion_queue = &motion.get_queue();
    z_queues.tmc2130_driver_queue = &tmc2130_driver.get_queue();
    z_queues.move_group_queue = &move_group.get_queue();
    z_queues.move_status_report_queue = &move_status_reporter.get_queue();
    z_queues.spi_queue = &spi_task.get_queue();
#if PCBA_PRIMARY_REVISION != 'b'
    z_queues.z_usage_storage_queue = &z_usage_storage_task.get_queue();
#endif
    spi_task_client.set_queue(&spi_task.get_queue());

    return z_tasks::call_run_diag0_interrupt;
}

void z_tasks::call_run_diag0_interrupt() {
    if (get_all_tasks().motion_controller) {
        return get_all_tasks().motion_controller->run_diag0_interrupt();
    }
}

z_tasks::QueueClient::QueueClient()
    : can::message_writer::MessageWriter{can::ids::NodeId::gripper_z} {}

void z_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void z_tasks::QueueClient::send_motor_driver_queue(
    const tmc2130::tasks::TaskMessage& m) {
    tmc2130_driver_queue->try_write(m);
}

void z_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void z_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void z_tasks::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
    // rev b boards don't have a working eeprom so we need to ignore
    // messages in that case
    if (z_usage_storage_queue != nullptr) {
        z_usage_storage_queue->try_write(m);
    }
}
auto z_tasks::get_queues() -> z_tasks::QueueClient& { return z_queues; }
