#pragma once
#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "head/core/tasks/presence_sensing_driver_task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2130_motor_driver_task.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"

namespace head_tasks {

/**
 * Start head tasks.
 */

void start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        left_motion_controller,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        right_motion_controller,
    presence_sensing_driver::PresenceSensingDriver& presence_sensing_driver,
    spi::hardware::SpiDeviceBase& spi2_device,
    spi::hardware::SpiDeviceBase& spi3_device,
    tmc2130::configs::TMC2130DriverConfig& left_driver_configs,
    tmc2130::configs::TMC2130DriverConfig& right_driver_configs,
    motor_hardware_task::MotorHardwareTask& right_motor_hardware,
    motor_hardware_task::MotorHardwareTask& left_motor_hardware,
    i2c::hardware::I2CBase& i2c3,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface);

/**
 * Access to all tasks not associated with a motor. This will be a singleton.
 */
struct HeadTasks {
    can::message_writer_task::MessageWriterTask<
        freertos_message_queue::FreeRTOSMessageQueue>* can_writer{nullptr};
    presence_sensing_driver_task::PresenceSensingDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>*
        presence_sensing_driver_task{nullptr};
    i2c::tasks::I2CTask<freertos_message_queue::FreeRTOSMessageQueue>*
        i2c3_task{nullptr};
    i2c::tasks::I2CPollerTask<freertos_message_queue::FreeRTOSMessageQueue,
                              freertos_timer::FreeRTOSTimer>* i2c3_poller_task{
        nullptr};
    eeprom::task::EEPromTask<freertos_message_queue::FreeRTOSMessageQueue>*
        eeprom_task{nullptr};
    eeprom::data_rev_task::UpdateDataRevTask<
        freertos_message_queue::FreeRTOSMessageQueue>* update_data_rev_task{
        nullptr};
};

/**
 * Access to all tasks associated with a motor. There will be one for the left
 * and one for the right.
 */
struct MotorTasks {
    tmc2130::tasks::MotorDriverTask<
        freertos_message_queue::FreeRTOSMessageQueue>* tmc2130_driver{nullptr};
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
    usage_storage_task::UsageStorageTask<
        freertos_message_queue::FreeRTOSMessageQueue>* usage_storage_task{
        nullptr};
};

/**
 * Access to the head tasks singleton
 * @return
 */
[[nodiscard]] auto get_tasks() -> HeadTasks&;

/**
 * Access to the right tasks singleton
 * @return
 */
[[nodiscard]] auto get_right_tasks() -> MotorTasks&;

/**
 * Access to the left tasks singleton
 * @return
 */
[[nodiscard]] auto get_left_tasks() -> MotorTasks&;

}  // namespace head_tasks
