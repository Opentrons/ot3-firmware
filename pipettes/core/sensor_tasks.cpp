#include "pipettes/core/sensor_tasks.hpp"

#include "can/core/ids.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/pipette_type.h"

static auto tasks = sensor_tasks::Tasks{};
static auto queue_client = sensor_tasks::QueueClient{};
static std::array<float, PRESSURE_SENSOR_BUFFER_SIZE> p_buff;
static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto environment_sensor_task_builder =
    freertos_task::TaskStarter<512, sensors::tasks::EnvironmentSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto capacitive_sensor_task_builder_rear =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto capacitive_sensor_task_builder_front =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto pressure_sensor_task_builder_rear =
    freertos_task::TaskStarter<512, sensors::tasks::PressureSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto pressure_sensor_task_builder_front =
    freertos_task::TaskStarter<512, sensors::tasks::PressureSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S1);

static auto tip_notification_task_builder_rear =
    freertos_task::TaskStarter<256, sensors::tasks::TipPresenceNotificationTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto tip_notification_task_builder_front =
    freertos_task::TaskStarter<256, sensors::tasks::TipPresenceNotificationTask,
                               can::ids::SensorId>(can::ids::SensorId::S1);

void sensor_tasks::start_tasks(
    sensor_tasks::CanWriterTask& can_writer,
    sensor_tasks::I2CClient& i2c3_task_client,
    sensor_tasks::I2CPollerClient& i2c3_poller_client,
    sensor_tasks::I2CClient& i2c2_task_client,
    sensor_tasks::I2CPollerClient& i2c2_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware_primary,
    can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware,
    sensors::mmr920::SensorVersion sensor_version) {
    // Low throughput sensor task (single channel)
    queue_client.set_node_id(id);
    auto& queues = sensor_tasks::get_queues();
    auto& tasks = sensor_tasks::get_tasks();

    auto& pressure_i2c_client = get_pipette_type() == EIGHT_CHANNEL
                                    ? i2c2_task_client
                                    : i2c3_task_client;
    auto& pressure_i2c_poller = get_pipette_type() == EIGHT_CHANNEL
                                    ? i2c2_poller_client
                                    : i2c3_poller_client;

    auto& eeprom_i2c_client = get_pipette_type() == NINETY_SIX_CHANNEL
                                  ? i2c3_task_client
                                  : i2c2_task_client;

    auto& eeprom_task = eeprom_task_builder.start(
        5, "eeprom", eeprom_i2c_client, eeprom_hardware);
    auto& environment_sensor_task = environment_sensor_task_builder.start(
        5, "enviro sensor", i2c3_task_client, i2c3_poller_client, queues);
    auto& pressure_sensor_task_rear = pressure_sensor_task_builder_rear.start(
        5, "pressure sensor s0", pressure_i2c_client, pressure_i2c_poller,
        queues, sensor_hardware_primary, sensor_version, p_buff);
    auto& capacitive_sensor_task_rear =
        capacitive_sensor_task_builder_rear.start(
            5, "capacitive sensor s0", i2c3_task_client, i2c3_poller_client,
            sensor_hardware_primary, queues);
    auto& tip_notification_task_rear = tip_notification_task_builder_rear.start(
        5, "tip notification sensor s0", queues, sensor_hardware_primary);

    tasks.eeprom_task = &eeprom_task;
    tasks.environment_sensor_task = &environment_sensor_task;
    tasks.capacitive_sensor_task_rear = &capacitive_sensor_task_rear;
    tasks.pressure_sensor_task_rear = &pressure_sensor_task_rear;
    tasks.tip_notification_task_rear = &tip_notification_task_rear;

    queues.set_queue(&can_writer.get_queue());
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.environment_sensor_queue = &environment_sensor_task.get_queue();
    queues.capacitive_sensor_queue_rear =
        &capacitive_sensor_task_rear.get_queue();
    queues.pressure_sensor_queue_rear = &pressure_sensor_task_rear.get_queue();
    queues.tip_notification_queue_rear =
        &tip_notification_task_rear.get_queue();
}

void sensor_tasks::start_tasks(
    sensor_tasks::CanWriterTask& can_writer,
    sensor_tasks::I2CClient& i2c3_task_client,
    sensor_tasks::I2CPollerClient& i2c3_poller_client,
    sensor_tasks::I2CClient& i2c2_task_client,
    sensor_tasks::I2CPollerClient& i2c2_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware_primary,
    sensors::hardware::SensorHardwareBase& sensor_hardware_secondary,
    can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware,
    sensors::mmr920::SensorVersion sensor_version) {
    // High throughput sensor task (eight and ninety six channel)
    queue_client.set_node_id(id);
    auto& queues = sensor_tasks::get_queues();
    auto& tasks = sensor_tasks::get_tasks();

    auto PIPETTE_TYPE = get_pipette_type();

    auto& primary_pressure_i2c_client =
        PIPETTE_TYPE == EIGHT_CHANNEL ? i2c2_task_client : i2c3_task_client;
    auto& primary_pressure_i2c_poller =
        PIPETTE_TYPE == EIGHT_CHANNEL ? i2c2_poller_client : i2c3_poller_client;

    auto& secondary_pressure_i2c_client =
        PIPETTE_TYPE == EIGHT_CHANNEL ? i2c3_task_client : i2c2_task_client;
    auto& secondary_pressure_i2c_poller =
        PIPETTE_TYPE == EIGHT_CHANNEL ? i2c3_poller_client : i2c2_poller_client;

    auto& eeprom_i2c_client = PIPETTE_TYPE == NINETY_SIX_CHANNEL
                                  ? i2c3_task_client
                                  : i2c2_task_client;
    auto shared_cap_task = PIPETTE_TYPE == EIGHT_CHANNEL ? true : false;
    auto front_tip_presence_sensor =
        PIPETTE_TYPE == NINETY_SIX_CHANNEL ? true : false;

    auto& eeprom_task = eeprom_task_builder.start(
        5, "eeprom", eeprom_i2c_client, eeprom_hardware);
    auto& environment_sensor_task = environment_sensor_task_builder.start(
        5, "enviro sensor", i2c3_task_client, i2c3_poller_client, queues);
    auto& pressure_sensor_task_rear = pressure_sensor_task_builder_rear.start(
        5, "pressure sensor s0", primary_pressure_i2c_client,
        primary_pressure_i2c_poller, queues, sensor_hardware_primary,
        sensor_version, p_buff);
    auto& pressure_sensor_task_front = pressure_sensor_task_builder_front.start(
        5, "pressure sensor s1", secondary_pressure_i2c_client,
        secondary_pressure_i2c_poller, queues, sensor_hardware_secondary,
        sensor_version, p_buff);
    auto& capacitive_sensor_task_rear =
        capacitive_sensor_task_builder_rear.start(
            5, "capacitive sensor s0", i2c3_task_client, i2c3_poller_client,
            sensor_hardware_primary, queues, shared_cap_task);
    auto& tip_notification_task_rear = tip_notification_task_builder_rear.start(
        5, "tip notification sensor s0", queues, sensor_hardware_primary);

    tasks.eeprom_task = &eeprom_task;
    tasks.environment_sensor_task = &environment_sensor_task;
    tasks.pressure_sensor_task_rear = &pressure_sensor_task_rear;
    tasks.pressure_sensor_task_front = &pressure_sensor_task_front;
    tasks.tip_notification_task_rear = &tip_notification_task_rear;
    tasks.capacitive_sensor_task_rear = &capacitive_sensor_task_rear;

    queues.set_queue(&can_writer.get_queue());
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.environment_sensor_queue = &environment_sensor_task.get_queue();
    queues.capacitive_sensor_queue_rear =
        &capacitive_sensor_task_rear.get_queue();
    queues.pressure_sensor_queue_rear = &pressure_sensor_task_rear.get_queue();
    queues.pressure_sensor_queue_front =
        &pressure_sensor_task_front.get_queue();
    queues.tip_notification_queue_rear =
        &tip_notification_task_rear.get_queue();

    if (shared_cap_task) {
        // There is only one cap sensor on the eight channel and so the "front"
        // and "rear" nozzles are actually supported by 1 single task so the
        // 'front'/'rear' should be set to the same queue/task.
        tasks.capacitive_sensor_task_front = &capacitive_sensor_task_rear;
        queues.capacitive_sensor_queue_front =
            &capacitive_sensor_task_rear.get_queue();
    } else {
        auto& capacitive_sensor_task_front =
            capacitive_sensor_task_builder_front.start(
                5, "capacitive sensor s1", i2c2_task_client, i2c2_poller_client,
                sensor_hardware_primary, queues);
        tasks.capacitive_sensor_task_front = &capacitive_sensor_task_front;
        queues.capacitive_sensor_queue_front =
            &capacitive_sensor_task_front.get_queue();
    }
    if (front_tip_presence_sensor) {
        // the eight channel only has one tip presence sensor, so the front
        // task should only be started if we have a 96 channel pipette
        auto& tip_notification_task_front =
            tip_notification_task_builder_front.start(
                5, "tip notification sensor s1", queues,
                sensor_hardware_secondary);
        tasks.tip_notification_task_front = &tip_notification_task_front;
        queues.tip_notification_queue_front =
            &tip_notification_task_front.get_queue();
    }
}

sensor_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

void sensor_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void sensor_tasks::QueueClient::send_environment_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    environment_sensor_queue->try_write(m);
}

void sensor_tasks::QueueClient::send_capacitive_sensor_queue_rear(
    const sensors::utils::TaskMessage& m) {
    capacitive_sensor_queue_rear->try_write(m);
}

void sensor_tasks::QueueClient::send_capacitive_sensor_queue_front(
    const sensors::utils::TaskMessage& m) {
    if (capacitive_sensor_queue_front != nullptr) {
        capacitive_sensor_queue_front->try_write(m);
    }
}

void sensor_tasks::QueueClient::send_pressure_sensor_queue_rear(
    const sensors::utils::TaskMessage& m) {
    pressure_sensor_queue_rear->try_write(m);
}

void sensor_tasks::QueueClient::send_pressure_sensor_queue_front(
    const sensors::utils::TaskMessage& m) {
    // The single channel only has 1 pressure sensor which
    // is generally referred to as the "rear". In this instance,
    // the front queue should not be dereferenced and
    // we should double check this by making sure the
    // front queue is not a nullptr.
    if (pressure_sensor_queue_front != nullptr) {
        pressure_sensor_queue_front->try_write(m);
    }
}

void sensor_tasks::QueueClient::send_tip_notification_queue_rear(
    const sensors::tip_presence::TaskMessage& m) {
    if (tip_notification_queue_rear != nullptr) {
        tip_notification_queue_rear->try_write(m);
    }
}

void sensor_tasks::QueueClient::send_tip_notification_queue_front(
    const sensors::tip_presence::TaskMessage& m) {
    if (tip_notification_queue_front != nullptr) {
        tip_notification_queue_front->try_write(m);
    }
}

auto sensor_tasks::get_tasks() -> Tasks& { return tasks; }

auto sensor_tasks::get_queues() -> QueueClient& { return queue_client; }
