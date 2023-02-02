#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace gantry {
namespace tasks {

using DefinedMotorHardware = motor_hardware::MotorHardware<motor_hardware::HardwareConfig>;

/**
 * Start gantry tasks.
 */
void start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig, DefinedMotorHardware>& motion_controller,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2160::configs::TMC2160DriverConfig& driver_configs,
    motor_hardware_task::MotorHardwareTask<DefinedMotorHardware>& mh_tsk);

/**
 * Access to all tasks in the system.
 */
struct AllTask {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    tmc2160::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* tmc2160_driver{nullptr};
    motion_controller_task::MotionControllerTask<
        freertos_message_queue::FreeRTOSMessageQueue>* motion_controller{
        nullptr};
    move_status_reporter_task::MoveStatusReporterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_status_reporter{
        nullptr};
    move_group_task::MoveGroupTask<
        freertos_message_queue::FreeRTOSMessageQueue>* move_group{nullptr};
    spi::tasks::Task<freertos_message_queue::FreeRTOSMessageQueue>* spi_task{
        nullptr};
};

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> AllTask&;

};  // namespace tasks
};  // namespace gantry
