#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"
#include "sensors/core/fdc1004.hpp"
#include "sensors/core/utils.hpp"

namespace capacitive_sensor_task {

using namespace fdc1004_utils;
using namespace can_ids;


template <message_writer_task::TaskClient CanClient>
struct ReadCapacitanceCallback {
    CanClient &can_client;
    float current_offset;
    uint32_t measurement = 0;
    uint16_t number_of_reads = 1;

    void operator()(const pipette_messages::MaxMessageBuffer &MSB_buffer, const pipette_messages::MaxMessageBuffer &LSB_buffer, bool final_read) {
        uint16_t msb_data = 0x0;
        uint16_t lsb_data = 0x0;
        const auto *MSB_iter = MSB_buffer.cbegin();
        MSB_iter = bit_utils::bytes_to_int(MSB_iter, MSB_buffer.cend(), msb_data);
        const auto *LSB_iter = LSB_buffer.cbegin();
        LSB_iter = bit_utils::bytes_to_int(LSB_iter, LSB_buffer.cend(), lsb_data);
        measurement += (msb_data << MSB_SHIFT | lsb_data);

        if (final_read) {
            auto capacitive = convert(measurement, number_of_reads, current_offset);
            auto message = can_messages::ReadFromSensorResponse{{}, SensorType::capacitive, capacitive};
            can_client.send_can_message(can_ids::NodeId::host, message);
        }
    }

    void reset() {
        number_of_reads = 1;
        measurement = 0;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        number_of_reads = number_of_reads;
    }

    void set_offset(float new_offset) {
        current_offset = new_offset;
    }
};

template <message_writer_task::TaskClient CanClient>
struct ReadOffsetCallback {
    CanClient &can_client;
    float current_offset;
    uint32_t measurement = 0;
    uint16_t number_of_reads = 1;

    void operator()(const pipette_messages::MaxMessageBuffer &buffer, bool final_read) {
        uint16_t data = 0x0;
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);

        if (final_read) {
            auto offset = calculate_capdac(measurement, number_of_reads, current_offset);
            auto message = can_messages::ReadFromSensorResponse{{}, SensorType::capacitive, offset};
            can_client.send_can_message(can_ids::NodeId::host, message);
        }
    }

    void reset() {
        measurement = 0;
        number_of_reads = 1;
    }

    void set_number_of_reads(uint16_t number_of_reads) {
        number_of_reads = number_of_reads;
    }

    void set_offset(float new_offset) {
        current_offset = new_offset;
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

    uint16_t get_storage() {
        return storage[0];
    }
};

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class CapacitiveMessageHandler {
  public:
    explicit CapacitiveMessageHandler(I2CQueueWriter &i2c_writer,
                                      CanClient &can_client)
        : writer{i2c_writer}, can_client{can_client}, capacitance_callback{can_client, CAPDAC_OFFSET}, capdac_callback{can_client, CAPDAC_OFFSET} {}
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
        writer.write(DEVICE_ID, ADDRESS);
        writer.read(ADDRESS, internal_callback, DEVICE_ID);
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        uint16_t configuration_data = CONFIGURATION_MEASUREMENT << 8 | DEVICE_CONFIGURATION;
        writer.write(configuration_data, ADDRESS);
        configuration_data = FDC_CONFIGURATION << 8 | SAMPLE_RATE;
        writer.write(configuration_data, ADDRESS);
    }

  private:

    void visit(can_messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (m.offset_reading) {
            capdac_callback.reset();
            capdac_callback.set_offset(CAPDAC_OFFSET);
            writer.write(CONFIGURATION_MEASUREMENT, ADDRESS);
            writer.read(ADDRESS, capdac_callback, CONFIGURATION_MEASUREMENT);
        } else {
            capacitance_callback.reset();
            capacitance_callback.set_offset(CAPDAC_OFFSET);
            writer.multi_register_poll(ADDRESS, 1, capacitance_callback, MSB_MEASUREMENT_1, LSB_MEASUREMENT_1);
        }

    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        writer.write(m.data, ADDRESS);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (m.offset_update) {
            capdac_callback.reset();
            capdac_callback.set_number_of_reads(m.sample_rate);
            capdac_callback.set_offset(CAPDAC_OFFSET);
            writer.single_register_poll(ADDRESS, m.sample_rate, capdac_callback, CONFIGURATION_MEASUREMENT);
        } else {
            capacitance_callback.reset();
            capacitance_callback.set_number_of_reads(m.sample_rate);
            capacitance_callback.set_offset(CAPDAC_OFFSET);
            writer.multi_register_poll(ADDRESS, m.sample_rate, capacitance_callback, MSB_MEASUREMENT_1, LSB_MEASUREMENT_1);
        }
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor\n", m.threshold, m.sensor);
        THRESHOLD = m.threshold;
        auto message = can_messages::SensorThresholdResponse{{}, THRESHOLD};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    InternalCallback internal_callback{};
    sensor_task_utils::BitMode mode = sensor_task_utils::BitMode::MSB;
    // 8 pF
    uint32_t THRESHOLD = 0x8;
    float CAPDAC_OFFSET = 0x0;
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
requires MessageQueue<QueueImpl<sensor_task_utils::TaskMessage>, sensor_task_utils::TaskMessage>
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