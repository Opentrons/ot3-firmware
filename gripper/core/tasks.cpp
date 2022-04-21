#include "gripper/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "gripper/core/can_task.hpp"

#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto tasks = gripper_tasks::AllTask{};
static auto queues = gripper_tasks::QueueClient{can_ids::NodeId::gripper};

static auto spi_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2130::tasks::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};
static auto brushed_motor_driver_task_builder =
    freertos_task::TaskStarter<512,
                               brushed_motor_driver_task::MotorDriverTask>{};
static auto brushed_motion_controller_task_builder = freertos_task::TaskStarter<
    512, brushed_motion_controller_task::MotionControllerTask>{};

static auto spi_task_builder = freertos_task::TaskStarter<512, spi::tasks::SPITask>{};

/**
 * Start gripper tasks.
 */
void gripper_tasks::start_tasks(
    can_bus::CanBus& can_bus, motor_class::Motor<lms::LeadScrewConfig>& z_motor,
    brushed_motor::BrushedMotor& grip_motor,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    auto& motion =
        mc_task_builder.start(5, "z mc", z_motor.motion_controller, queues);
    auto& motor = motor_driver_task_builder.start(5, "motor driver",
                                                  z_motor.driver, queues);
    auto& move_group =
        move_group_task_builder.start(5, "move group", queues, queues);
    auto& tmc2130_driver = motor_driver_task_builder.start(
        5, "tmc2130 driver", driver_configs, queues, spi_task_client);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", queues,
        z_motor.motion_controller.get_mechanical_config());
    auto& brushed_motor = brushed_motor_driver_task_builder.start(
        5, "bdc driver", grip_motor.driver, queues);
    auto& brushed_motion = brushed_motion_controller_task_builder.start(
        5, "bdc controller", grip_motor.motion_controller, queues);

    tasks.can_writer = &can_writer;
    tasks.motion_controller = &motion;
    tasks.tmc2130_driver = &tmc2130_driver;
    tasks.move_group = &move_group;
    tasks.move_status_reporter = &move_status_reporter;
    tasks.brushed_motor_driver = &brushed_motor;
    tasks.brushed_motion_controller = &brushed_motion;

    queues.motion_queue = &motion.get_queue();
    queues.tmc2130_driver_queue = &tmc2130_driver.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.set_queue(&can_writer.get_queue());
    queues.move_status_report_queue = &move_status_reporter.get_queue();
    queues.brushed_motor_queue = &brushed_motor.get_queue();
    queues.brushed_motion_queue = &brushed_motion.get_queue();
}

gripper_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

void gripper_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_motor_driver_queue(
    const tmc2130::tasks::TaskMessage& m) {
    tmc2130_driver_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void gripper_tasks::QueueClient::send_brushed_motor_driver_queue(
    const brushed_motor_driver_task::TaskMessage& m) {
    brushed_motor_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_brushed_motion_controller_queue(
    const brushed_motion_controller_task::TaskMessage& m) {
    brushed_motion_queue->try_write(m);
}

/**
 * Access to the tasks singleton
 * @return
 */
auto gripper_tasks::get_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gripper_tasks::get_queues() -> QueueClient& { return queues; }
