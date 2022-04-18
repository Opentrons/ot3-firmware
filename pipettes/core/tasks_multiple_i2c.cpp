#include "can/core/ids.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/tasks/i2c_poller_task_starter.hpp"
#include "i2c/core/tasks/i2c_task_starter.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/tasks/motion_controller_task_starter.hpp"
#include "motor-control/core/tasks/motor_driver_task_starter.hpp"
#include "motor-control/core/tasks/move_group_task_starter.hpp"
#include "motor-control/core/tasks/move_status_reporter_task_starter.hpp"
#include "pipettes/core/can_task.hpp"
#include "pipettes/core/tasks.hpp"
#include "pipettes/core/tasks/eeprom_task_starter.hpp"
#include "sensors/core/tasks/environmental_sensor_task_starter.hpp"

static auto tasks = pipettes_tasks::AllTask{};
static auto queue_client = pipettes_tasks::QueueClient{};
static auto i2c1_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto mc_task_builder =
    motion_controller_task_starter::TaskStarter<lms::LeadScrewConfig, 512,
                                                pipettes_tasks::QueueClient>{};
static auto motor_driver_task_builder =
    motor_driver_task_starter::TaskStarter<512, pipettes_tasks::QueueClient>{};
static auto move_group_task_builder =
    move_group_task_starter::TaskStarter<512, pipettes_tasks::QueueClient,
                                         pipettes_tasks::QueueClient>{};
static auto move_status_task_builder =
    move_status_reporter_task_starter::TaskStarter<
        512, pipettes_tasks::QueueClient, lms::LeadScrewConfig>{};
static auto eeprom_task_builder =
    eeprom_task_starter::TaskStarter<512, pipettes_tasks::QueueClient>{};

static auto environment_sensor_task_builder =
    sensors::tasks::EnvironmentalSensorTaskStarter<
        512, pipettes_tasks::QueueClient>{};
static auto capacitive_sensor_task_builder =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask>{};
static auto pressure_sensor_task_builder =
    freertos_task::TaskStarter<512, sensors::tasks::PressureSensorTask>{};

static auto i2c_task_builder = i2c::task_starters::I2CTaskStarter<512>{};
static auto i2c_poll_task_builder =
    i2c::task_starters::PollerTaskStarter<1024>{};
static auto i2c1_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

/**
 * Start pipettes tasks.
 */
void pipettes_tasks::start_tasks(
    can_bus::CanBus& can_bus,
    motion_controller::MotionController<lms::LeadScrewConfig>&
        motion_controller,
    motor_driver::MotorDriver& motor_driver,
    i2c::hardware::I2CDeviceBase& i2c3_device,
    i2c::hardware::I2CDeviceBase& i2c1_device,
    sensors::hardware::SensorHardwareBase& sensor_hardware,
    can_ids::NodeId id) {
    queue_client.set_node_id(id);
    auto& queues = pipettes_tasks::get_queues();
    auto& tasks = pipettes_tasks::get_tasks();

    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus, id);
    auto& i2c3_task = i2c_task_builder.start(5, i2c3_device);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());
    auto& i2c1_task = i2c_task_builder.start(5, i2c1_device);
    i2c1_task_client.set_queue(&i2c1_task.get_queue());

    auto& i2c3_poller_task = i2c_poll_task_builder.start(5, i2c3_task_client);
    auto& i2c1_poller_task = i2c_poll_task_builder.start(5, i2c1_task_client);
    i2c1_poll_client.set_queue(&i2c1_poller_task.get_queue());
    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& motion = mc_task_builder.start(5, motion_controller, queues);
    auto& motor = motor_driver_task_builder.start(5, motor_driver, queues);
    auto& move_group = move_group_task_builder.start(5, queues, queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, queues, motion_controller.get_mechanical_config());

    auto& eeprom_task = eeprom_task_builder.start(5, i2c3_task_client, queues);
    auto& environment_sensor_task =
        environment_sensor_task_builder.start(5, i2c3_task_client, queues);
    auto& pressure_sensor_task = pressure_sensor_task_builder.start(
        5, "pressure sensor", i2c3_task_client, i2c3_poll_client, queues);
    auto& capacitive_sensor_task = capacitive_sensor_task_builder.start(
        5, "capacitive sensor", i2c3_task_client, i2c3_poll_client,
        sensor_hardware, queues);

    // TODO (lc: 03-21-2022, add necessary sensor tasks for secondary i2c bus
    tasks.can_writer = &can_writer;
    tasks.motion_controller = &motion;
    tasks.motor_driver = &motor;
    tasks.move_group = &move_group;
    tasks.move_status_reporter = &move_status_reporter;
    tasks.eeprom_task = &eeprom_task;
    tasks.environment_sensor_task = &environment_sensor_task;
    tasks.capacitive_sensor_task = &capacitive_sensor_task;
    tasks.pressure_sensor_task = &pressure_sensor_task;
    tasks.i2c3_task = &i2c3_task;
    tasks.i2c1_task = &i2c1_task;
    tasks.i2c3_poller_task = &i2c3_poller_task;
    tasks.i2c1_poller_task = &i2c1_poller_task;

    queues.motion_queue = &motion.get_queue();
    queues.motor_queue = &motor.get_queue();
    queues.move_group_queue = &move_group.get_queue();
    queues.set_queue(&can_writer.get_queue());
    queues.move_status_report_queue = &move_status_reporter.get_queue();
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.environment_sensor_queue = &environment_sensor_task.get_queue();
    queues.capacitive_sensor_queue = &capacitive_sensor_task.get_queue();
    queues.pressure_sensor_queue = &pressure_sensor_task.get_queue();

    queues.i2c3_queue = &i2c3_task.get_queue();
    queues.i2c1_queue = &i2c1_task.get_queue();
    queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    queues.i2c1_poller_queue = &i2c1_poller_task.get_queue();
}

pipettes_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can_message_writer::MessageWriter{can_ids::NodeId::pipette_left} {}

void pipettes_tasks::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_motor_driver_queue(
    const motor_driver_task::TaskMessage& m) {
    motor_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void pipettes_tasks::QueueClient::send_eeprom_queue(
    const eeprom_task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_environment_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    environment_sensor_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_capacitive_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    capacitive_sensor_queue->try_write(m);
}

void pipettes_tasks::QueueClient::send_pressure_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    pressure_sensor_queue->try_write(m);
}

/**
 * Access to the tasks singleton
 * @return
 */
auto pipettes_tasks::get_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto pipettes_tasks::get_queues() -> QueueClient& { return queue_client; }
