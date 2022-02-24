#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"
#include "pipettes/core/messages.hpp"
#include "sensors/core/hdc2080.hpp"
#include "sensors/core/utils.hpp"

namespace environment_sensor_task {

using namespace hdc2080_utils;
using namespace can_ids;

template <message_writer_task::TaskClient CanClient>
struct HumidityReadingCallback {
    CanClient &can_client;

    void operator()(const pipette_messages::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        auto humidity = convert(data, SensorType::humidity);

        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::humidity, static_cast<uint32_t>(humidity)};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }
};

template <message_writer_task::TaskClient CanClient>
struct TemperatureReadingCallback {
    CanClient &can_client;

    void operator()(const pipette_messages::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        auto temperature = convert(data, SensorType::temperature);

        auto message = can_messages::ReadFromSensorResponse{
            {}, SensorType::temperature, static_cast<uint32_t>(temperature)};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }
};

// This struct should be used when the message handler
// class receives information it should handle (i.e. the device info)
struct InternalCallback {
    std::array<uint16_t, 1> storage{};

    void operator()(const pipette_messages::MaxMessageBuffer &buffer) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
        storage[0] = data;
    }
};

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
        writer.read(ADDRESS, internal_callback, DEVICE_ID_REGISTER);
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
            writer.read(ADDRESS, humidity_callback, HUMIDITY_REGISTER);
        } else {
            writer.write(TEMPERATURE_REGISTER, ADDRESS);
            writer.read(ADDRESS, temperature_callback, TEMPERATURE_REGISTER);
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