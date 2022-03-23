#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
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
          capacitance_handler{can_client, writer, zero_threshold,
                              capdac_offset} {}
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
        writer.read(
            ADDRESS, [this]() { internal_callback.send_to_can(); },
            [this](auto message_a) {
                internal_callback.handle_data(message_a);
            },
            DEVICE_ID_REGISTER);
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.

        // MSB mode
        uint32_t configuration_data =
            CONFIGURATION_MEASUREMENT | (MSB_DEVICE_CONFIGURATION << 8);
        writer.write(configuration_data, ADDRESS);
        configuration_data = FDC_CONFIGURATION | (MSB_SAMPLE_RATE << 8);
        writer.write(configuration_data, ADDRESS);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(can_messages::ReadFromSensorRequest &m) {
        /**
         * The FDC1004 sensor has an offset register and
         * a measurement register. The CAPDAC offset is used
         * to account for any additional baseline noise that
         * may occur on the capacitve sensor.
         *
         */
        LOG("Received request to read from %d sensor", m.sensor);
        capdac_offset = capacitance_handler.get_offset();
        if (bool(m.offset_reading)) {
            auto message = can_messages::ReadFromSensorResponse{
                .sensor = SensorType::capacitive,
                .sensor_data = static_cast<int32_t>(capdac_offset)};
            can_client.send_can_message(can_ids::NodeId::host, message);
        } else {
            capacitance_handler.reset();
            writer.multi_register_poll(
                ADDRESS, 1, DELAY,
                [this]() { capacitance_handler.send_to_can(); },
                [this](auto message_a, auto message_b) {
                    capacitance_handler.handle_data(message_a, message_b);
                },
                MSB_MEASUREMENT_1, LSB_MEASUREMENT_1);
        }
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        writer.write(m.data, ADDRESS);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor", m.sensor);
        capdac_offset = capacitance_handler.get_offset();
        capacitance_handler.reset();
        capacitance_handler.set_number_of_reads(m.sample_rate);
        writer.multi_register_poll(
            ADDRESS, m.sample_rate, DELAY,
            [this]() { capacitance_handler.send_to_can(); },
            [this](auto message_a, auto message_b) {
                capacitance_handler.handle_data(message_a, message_b);
            },
            MSB_MEASUREMENT_1, LSB_MEASUREMENT_1);
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor",
            m.threshold, m.sensor);
        zero_threshold = m.threshold;
        auto message = can_messages::SensorThresholdResponse{
            .sensor = SensorType::capacitive, .threshold = zero_threshold};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    InternalCallback internal_callback{};
    sensor_task_utils::BitMode mode = sensor_task_utils::BitMode::MSB;
    // 3 pF
    int32_t zero_threshold = 0x3;
    // 0 pF
    float capdac_offset = 0x0;
    static constexpr uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    CanClient &can_client;
    ReadCapacitanceCallback<CanClient, I2CQueueWriter> capacitance_handler;
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