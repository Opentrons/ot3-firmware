#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"
#include "sensors/core/tasks/capacitive_sensor_callbacks.hpp"
#include "sensors/core/utils.hpp"

namespace capacitive_sensor_task {

using namespace capacitance_callbacks;

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class CapacitiveMessageHandler {
  public:
    explicit CapacitiveMessageHandler(I2CQueueWriter &i2c_writer,
                                      CanClient &can_client)
        : writer{i2c_writer},
          can_client{can_client},
          capacitance_callback{can_client, CAPDAC_OFFSET},
          capdac_callback{can_client, CAPDAC_OFFSET} {}
    CapacitiveMessageHandler(const CapacitiveMessageHandler &) = delete;
    CapacitiveMessageHandler(const CapacitiveMessageHandler &&) = delete;
    auto operator=(const CapacitiveMessageHandler &)
        -> CapacitiveMessageHandler & = delete;
    auto operator=(const CapacitiveMessageHandler &&)
        -> CapacitiveMessageHandler && = delete;
    ~CapacitiveMessageHandler() = default;

    void handle_message(sensor_task_utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void initialize() {
        writer.write(DEVICE_ID_REGISTER, ADDRESS);
        writer.read(ADDRESS, &internal_callback, DEVICE_ID_REGISTER);
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        uint16_t configuration_data =
            CONFIGURATION_MEASUREMENT << 8 | DEVICE_CONFIGURATION;
        writer.write(configuration_data, ADDRESS);
        configuration_data = FDC_CONFIGURATION << 8 | SAMPLE_RATE;
        writer.write(configuration_data, ADDRESS);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(can_messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (bool(m.offset_reading)) {
            capdac_callback.reset();
            capdac_callback.set_offset(CAPDAC_OFFSET);
            writer.write(CONFIGURATION_MEASUREMENT, ADDRESS);
            writer.read(ADDRESS, &capdac_callback, CONFIGURATION_MEASUREMENT);
        } else {
            capacitance_callback.reset();
            capacitance_callback.set_offset(CAPDAC_OFFSET);
            writer.multi_register_poll(ADDRESS, 1, DELAY, &capacitance_callback,
                                       MSB_MEASUREMENT_1, LSB_MEASUREMENT_1);
        }
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        writer.write(m.data, ADDRESS);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (bool(m.offset_update)) {
            capdac_callback.reset();
            capdac_callback.set_number_of_reads(m.sample_rate);
            capdac_callback.set_offset(CAPDAC_OFFSET);
            writer.single_register_poll(ADDRESS, m.sample_rate, DELAY,
                                        &capdac_callback,
                                        CONFIGURATION_MEASUREMENT);
        } else {
            capacitance_callback.reset();
            capacitance_callback.set_number_of_reads(m.sample_rate);
            capacitance_callback.set_offset(CAPDAC_OFFSET);
            writer.multi_register_poll(ADDRESS, m.sample_rate, DELAY,
                                       &capacitance_callback, MSB_MEASUREMENT_1,
                                       LSB_MEASUREMENT_1);
        }
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor\n",
            m.threshold, m.sensor);
        THRESHOLD = m.threshold;
        auto message = can_messages::SensorThresholdResponse{
            {}, SensorType::capacitive, THRESHOLD};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    InternalCallback internal_callback{};
    sensor_task_utils::BitMode mode = sensor_task_utils::BitMode::MSB;
    // 8 pF
    uint32_t THRESHOLD = 0x8;
    float CAPDAC_OFFSET = 0x0;
    uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    CanClient &can_client;
    ReadCapacitanceCallback<CanClient> capacitance_callback;
    ReadOffsetCallback<CanClient> capdac_callback;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, class I2CQueueWriter,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<sensor_task_utils::TaskMessage>,
                      sensor_task_utils::TaskMessage>
class CapacitiveSensorTask {
  public:
    using QueueType = QueueImpl<sensor_task_utils::TaskMessage>;
    CapacitiveSensorTask(QueueType &queue) : queue{queue} {}
    CapacitiveSensorTask(const CapacitiveSensorTask &c) = delete;
    CapacitiveSensorTask(const CapacitiveSensorTask &&c) = delete;
    auto operator=(const CapacitiveSensorTask &c) = delete;
    auto operator=(const CapacitiveSensorTask &&c) = delete;
    ~CapacitiveSensorTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(I2CQueueWriter *writer,
                                 CanClient *can_client) {
        auto handler = CapacitiveMessageHandler{*writer, *can_client};
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

}  // namespace capacitive_sensor_task