#include "common/core/freertos_task.hpp"
#include "gripper/core/can_task.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/brushed_move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

using namespace gripper_tasks;

static auto g_queues = g_tasks::QueueClient{};

static auto brushed_motor_driver_task_builder =
    freertos_task::TaskStarter<512,
                               brushed_motor_driver_task::MotorDriverTask>{};
static auto brushed_motion_controller_task_builder = freertos_task::TaskStarter<
    512, brushed_motion_controller_task::MotionControllerTask>{};

static auto brushed_move_group_task_builder =
    freertos_task::TaskStarter<512, brushed_move_group_task::MoveGroupTask>{};

static auto brushed_move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

#if PCBA_PRIMARY_REVISION != 'b'
static auto jaw_usage_storage_task_builder =
    freertos_task::TaskStarter<512, usage_storage_task::UsageStorageTask>{};
#endif

void g_tasks::start_task(
    brushed_motor::BrushedMotor<lms::GearBoxConfig>& grip_motor,
    AllTask& gripper_tasks, gripper_tasks::QueueClient& main_queues) {
    auto& brushed_motor = brushed_motor_driver_task_builder.start(
        5, "bdc driver", grip_motor.driver, g_queues);
    auto& brushed_motion = brushed_motion_controller_task_builder.start(
        5, "bdc controller", grip_motor.motion_controller, g_queues, g_queues);
    auto& brushed_move_group = brushed_move_group_task_builder.start(
        5, "bdc move group", g_queues, g_queues);
    auto& brushed_move_status_reporter = brushed_move_status_task_builder.start(
        5, "bdc move status", g_queues,
        grip_motor.motion_controller.get_mechanical_config(), g_queues);
#if PCBA_PRIMARY_REVISION != 'b'
    auto& usage_storage_task = jaw_usage_storage_task_builder.start(
        5, "jaw usage storage", g_queues, main_queues);
#else
    std::ignore = main_queues;
#endif

    gripper_tasks.brushed_motor_driver = &brushed_motor;
    gripper_tasks.brushed_motion_controller = &brushed_motion;
    gripper_tasks.brushed_move_group = &brushed_move_group;
    gripper_tasks.brushed_move_status_reporter = &brushed_move_status_reporter;
#if PCBA_PRIMARY_REVISION != 'b'
    gripper_tasks.jaw_usage_storage_task = &usage_storage_task;
#endif
    g_queues.brushed_motor_queue = &brushed_motor.get_queue();
    g_queues.brushed_motion_queue = &brushed_motion.get_queue();
    g_queues.brushed_move_group_queue = &brushed_move_group.get_queue();
    g_queues.brushed_move_status_report_queue =
        &brushed_move_status_reporter.get_queue();
#if PCBA_PRIMARY_REVISION != 'b'
    g_queues.usage_storage_queue = &usage_storage_task.get_queue();
#endif
}

g_tasks::QueueClient::QueueClient()
    : can::message_writer::MessageWriter{can::ids::NodeId::gripper_g} {}

void g_tasks::QueueClient::send_brushed_motor_driver_queue(
    const brushed_motor_driver_task::TaskMessage& m) {
    brushed_motor_queue->try_write(m);
}

void g_tasks::QueueClient::send_brushed_motion_controller_queue(
    const brushed_motion_controller_task::TaskMessage& m) {
    brushed_motion_queue->try_write(m);
}

void g_tasks::QueueClient::send_brushed_move_group_queue(
    const brushed_move_group_task::TaskMessage& m) {
    brushed_move_group_queue->try_write(m);
}

void g_tasks::QueueClient::send_brushed_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(brushed_move_status_report_queue->try_write_isr(m));
}

void g_tasks::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
#if PCBA_PRIMARY_REVISION == 'b'
    // this task depends on the eeprom which does not work on the EVT gripper
    // Since we don't create the task make sure we don't try to write to a
    // non-existent queue
    std::ignore = m;
#else
    usage_storage_queue->try_write(m);
#endif
}
auto g_tasks::get_queues() -> g_tasks::QueueClient& { return g_queues; }
