#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"
#include "sensors/core/tasks/environment_sensor_callbacks.hpp"
#include "sensors/core/utils.hpp"

namespace environment_sensor_task {

using namespace environment_sensor_callbacks;

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class EnvironmentSensorMessageHandler {
  public:
    explicit EnvironmentSensorMessageHandler(I2CQueueWriter &i2c_writer,
                                             CanClient &can_client)
        : writer{i2c_writer},
          can_client{can_client},
          humidity_callback{can_client},
          temperature_callback{can_client} {}
    EnvironmentSensorMessageHandler(const EnvironmentSensorMessageHandler &) =
        delete;
    EnvironmentSensorMessageHandler(const EnvironmentSensorMessageHandler &&) =
        delete;
    auto operator=(const EnvironmentSensorMessageHandler &)
        -> EnvironmentSensorMessageHandler & = delete;
    auto operator=(const EnvironmentSensorMessageHandler &&)
        -> EnvironmentSensorMessageHandler && = delete;
    ~EnvironmentSensorMessageHandler() = default;

    void initialize() {
        writer.write(DEVICE_ID_REGISTER, ADDRESS);
        sensor_callbacks::SingleRegisterCallback callback{internal_callback};
        writer.read(ADDRESS, callback, callback, DEVICE_ID_REGISTER);
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        uint16_t configuration_data = DRDY_CONFIG << 8 | SAMPLE_RATE;
        writer.write(configuration_data, ADDRESS);
        configuration_data = INTERRUPT_REGISTER << 8 | SET_DATARDY;
        writer.write(configuration_data, ADDRESS);
        configuration_data =
            MEASURE_REGISTER << 8 | BEGIN_MEASUREMENT_RECORDING;
        writer.write(configuration_data, ADDRESS);
    }

    void handle_message(sensor_task_utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(can_messages::BaselineSensorRequest &m) {}

    void visit(can_messages::SetSensorThresholdRequest &m) {}

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        writer.write(m.data, ADDRESS);
    }

    void visit(can_messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (SensorType(m.sensor) == SensorType::humidity) {
            writer.write(HUMIDITY_REGISTER, ADDRESS);
            sensor_callbacks::SingleRegisterCallback callback{
                humidity_callback};
            writer.read(ADDRESS, callback, callback, HUMIDITY_REGISTER);
        } else {
            writer.write(TEMPERATURE_REGISTER, ADDRESS);
            sensor_callbacks::SingleRegisterCallback callback{
                temperature_callback};
            writer.read(ADDRESS, callback, callback, TEMPERATURE_REGISTER);
        }
    }

    InternalCallback internal_callback{};
    sensor_task_utils::BitMode mode = sensor_task_utils::BitMode::MSB;
    uint8_t HUMIDITY_REGISTER = LSB_HUMIDITY_REGISTER;
    uint8_t TEMPERATURE_REGISTER = LSB_TEMPERATURE_REGISTER;

    I2CQueueWriter &writer;
    CanClient &can_client;
    HumidityReadingCallback<CanClient> humidity_callback;
    TemperatureReadingCallback<CanClient> temperature_callback;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, class I2CQueueWriter,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<sensor_task_utils::TaskMessage>,
                      sensor_task_utils::TaskMessage>
class EnvironmentSensorTask {
  public:
    using QueueType = QueueImpl<sensor_task_utils::TaskMessage>;
    EnvironmentSensorTask(QueueType &queue) : queue{queue} {}
    EnvironmentSensorTask(const EnvironmentSensorTask &c) = delete;
    EnvironmentSensorTask(const EnvironmentSensorTask &&c) = delete;
    auto operator=(const EnvironmentSensorTask &c) = delete;
    auto operator=(const EnvironmentSensorTask &&c) = delete;
    ~EnvironmentSensorTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(I2CQueueWriter *writer,
                                 CanClient *can_client) {
        auto handler = EnvironmentSensorMessageHandler{*writer, *can_client};
        handler.initialize();
        sensor_task_utils::TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

}  // namespace environment_sensor_task