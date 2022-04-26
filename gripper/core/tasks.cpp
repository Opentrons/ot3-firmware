#include "gripper/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "gripper/core/can_task.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto tasks = gripper_tasks::AllTask{};
static auto queues = gripper_tasks::QueueClient{can_ids::NodeId::gripper};

static auto brushed_motor_driver_task_builder =
    freertos_task::TaskStarter<512,
                               brushed_motor_driver_task::MotorDriverTask>{};
static auto brushed_motion_controller_task_builder = freertos_task::TaskStarter<
    512, brushed_motion_controller_task::MotionControllerTask>{};

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
    tasks.can_writer = &can_writer;
    queues.set_queue(&can_writer.get_queue());

    z_tasks::start_task(z_motor, spi_device, driver_configs, tasks);

    g_tasks::start_task(grip_motor);
}

void gripper_tasks::g_tasks::start_task(
    brushed_motor::BrushedMotor& grip_motor) {
    auto& brushed_motor = brushed_motor_driver_task_builder.start(
        5, "bdc driver", grip_motor.driver, queues);
    auto& brushed_motion = brushed_motion_controller_task_builder.start(
        5, "bdc controller", grip_motor.motion_controller, queues);

    tasks.brushed_motor_driver = &brushed_motor;
    tasks.brushed_motion_controller = &brushed_motion;

    queues.brushed_motor_queue = &brushed_motor.get_queue();
    queues.brushed_motion_queue = &brushed_motion.get_queue();
}

gripper_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

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
