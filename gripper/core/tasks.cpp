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

    g_tasks::start_task(grip_motor, tasks);
}

gripper_tasks::QueueClient::QueueClient(can_ids::NodeId this_fw)
    : can_message_writer::MessageWriter{this_fw} {}

/**
 * Access to the tasks singleton
 * @return
 */
auto gripper_tasks::get_all_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gripper_tasks::get_main_queues() -> QueueClient& { return queues; }
