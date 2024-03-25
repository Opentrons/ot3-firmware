#include "gripper/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "gripper/core/can_task.hpp"
#include "gripper/firmware/eeprom_keys.hpp"
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

static auto i2c2_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c2_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};

template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;
static auto i2c2_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c3_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};

static auto i2c2_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

static auto capacitive_sensor_task_builder_front =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S1);

static auto capacitive_sensor_task_builder_rear =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto tail_accessor = eeprom::dev_data::DevDataTailAccessor{queues};
static auto eeprom_data_rev_update_builder =
    freertos_task::TaskStarter<512, eeprom::data_rev_task::UpdateDataRevTask>{};

/**
 * Start gripper tasks.
 */
auto gripper_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    motor_class::Motor<lms::LeadScrewConfig>& z_motor,
    brushed_motor::BrushedMotor<lms::GearBoxConfig>& grip_motor,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2130::configs::TMC2130DriverConfig& driver_configs,
    i2c::hardware::I2CBase& i2c2, i2c::hardware::I2CBase& i2c3,
    sensors::hardware::SensorHardwareBase& sensor_hardware,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface,
    motor_hardware_task::MotorHardwareTask& zmh_tsk,
    motor_hardware_task::MotorHardwareTask& gmh_tsk)
    -> z_motor_iface::diag0_handler {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    tasks.can_writer = &can_writer;
    queues.set_queue(&can_writer.get_queue());

    auto& i2c2_task = i2c2_task_builder.start(5, "i2c2", i2c2);
    i2c2_task_client.set_queue(&i2c2_task.get_queue());
    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());

    auto& i2c2_poller_task =
        i2c2_poll_task_builder.start(5, "i2c2 poller", i2c2_task_client);
    i2c2_poll_client.set_queue(&i2c2_poller_task.get_queue());
    auto& i2c3_poller_task =
        i2c3_poll_task_builder.start(5, "i2c3 poller", i2c3_task_client);
    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hw_iface);
#if PCBA_PRIMARY_REVISION != 'b'
    auto& eeprom_data_rev_update_task = eeprom_data_rev_update_builder.start(
        5, "data_rev_update", queues, tail_accessor, table_updater);
#endif
    auto& capacitive_sensor_task_front =
        capacitive_sensor_task_builder_front.start(
            5, "cap sensor S1", i2c2_task_client, i2c2_poll_client,
            sensor_hardware, queues);
    auto& capacitive_sensor_task_rear =
        capacitive_sensor_task_builder_rear.start(
            5, "cap sensor S0", i2c3_task_client, i2c3_poll_client,
            sensor_hardware, queues);

    tasks.i2c2_task = &i2c2_task;
    tasks.i2c3_task = &i2c3_task;
    tasks.i2c2_poller_task = &i2c2_poller_task;
    tasks.i2c3_poller_task = &i2c3_poller_task;
    tasks.eeprom_task = &eeprom_task;
    tasks.capacitive_sensor_task_front = &capacitive_sensor_task_front;
    tasks.capacitive_sensor_task_rear = &capacitive_sensor_task_rear;
#if PCBA_PRIMARY_REVISION != 'b'
    tasks.update_data_rev_task = &eeprom_data_rev_update_task;
#endif

    queues.i2c2_queue = &i2c2_task.get_queue();
    queues.i2c3_queue = &i2c3_task.get_queue();
    queues.i2c2_poller_queue = &i2c2_poller_task.get_queue();
    queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.capacitive_sensor_queue_front =
        &capacitive_sensor_task_front.get_queue();
    queues.capacitive_sensor_queue_rear =
        &capacitive_sensor_task_rear.get_queue();

    auto diag0_handler = z_tasks::start_task(
        z_motor, spi_device, driver_configs, tasks, queues, tail_accessor);

    g_tasks::start_task(grip_motor, tasks, queues, tail_accessor);

    z_tasks::get_queues().set_queue(&can_writer.get_queue());
    g_tasks::get_queues().set_queue(&can_writer.get_queue());

    zmh_tsk.start_task();
    gmh_tsk.start_task();

    return diag0_handler;
}

gripper_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void gripper_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void gripper_tasks::QueueClient::send_capacitive_sensor_queue_front(
    const sensors::utils::TaskMessage& m) {
    capacitive_sensor_queue_front->try_write(m);
}

void gripper_tasks::QueueClient::send_capacitive_sensor_queue_rear(
    const sensors::utils::TaskMessage& m) {
    capacitive_sensor_queue_rear->try_write(m);
}

// gripper does not have environment nor pressure sensor
void gripper_tasks::QueueClient::send_environment_sensor_queue(
    const sensors::utils::TaskMessage&) {}
void gripper_tasks::QueueClient::send_pressure_sensor_queue_front(
    const sensors::utils::TaskMessage&) {}
void gripper_tasks::QueueClient::send_pressure_sensor_queue_rear(
    const sensors::utils::TaskMessage&) {}

void gripper_tasks::QueueClient::send_tip_notification_queue_rear(
    const sensors::tip_presence::TaskMessage&) {}
void gripper_tasks::QueueClient::send_tip_notification_queue_front(
    const sensors::tip_presence::TaskMessage&) {}

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
