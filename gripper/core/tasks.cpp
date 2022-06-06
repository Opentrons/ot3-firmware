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
static auto queues = gripper_tasks::QueueClient{can::ids::NodeId::gripper};

static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};

/**
 * Start gripper tasks.
 */
void gripper_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    motor_class::Motor<lms::LeadScrewConfig>& z_motor,
    brushed_motor::BrushedMotor& grip_motor,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs,
    i2c::hardware::I2CBase& i2c3,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    tasks.can_writer = &can_writer;
    queues.set_queue(&can_writer.get_queue());

    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());
    queues.i2c3_queue = &i2c3_task.get_queue();

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hw_iface);
    tasks.eeprom_task = &eeprom_task;
    queues.eeprom_queue = &eeprom_task.get_queue();

    z_tasks::start_task(z_motor, spi_device, driver_configs, tasks);

    g_tasks::start_task(grip_motor, tasks);

    z_tasks::get_queues().set_queue(&can_writer.get_queue());
    g_tasks::get_queues().set_queue(&can_writer.get_queue());
}

gripper_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void gripper_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

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
