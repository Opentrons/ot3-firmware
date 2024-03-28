#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "gantry/core/interfaces_rev1.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

namespace gantry {
namespace tasks {

/**
 * Start gantry tasks.
 */
auto start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig>& motion_controller,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2160::configs::TMC2160DriverConfig& driver_configs,
    motor_hardware_task::MotorHardwareTask& mh_tsk,
    i2c::hardware::I2CBase& i2c2,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface)
    -> interfaces::diag0_handler;

void call_run_diag0_interrupt();

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
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c2_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c2_poller_task{
        nullptr};
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* usage_storage_task{
        nullptr};
    eeprom::data_rev_task::UpdateDataRevTask<
        freertos_message_queue::FreeRTOSMessageQueue>* update_data_rev_task{
        nullptr};
};

/**
 * Access to the tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> AllTask&;

};  // namespace tasks
};  // namespace gantry
