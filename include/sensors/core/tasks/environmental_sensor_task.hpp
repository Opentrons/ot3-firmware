#pragma once

#include <array>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/hdc2080.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <class I2CQueueWriter, can::message_writer_task::TaskClient CanClient,
          class OwnQueue>
class EnvironmentSensorMessageHandler {
  public:
    EnvironmentSensorMessageHandler(I2CQueueWriter &i2c_writer,
                                    CanClient &can_client, OwnQueue &own_queue,
                                    const can::ids::SensorId &id)
        : writer{i2c_writer},
          can_client{can_client},
          own_queue(own_queue),
          sensor_id(id) {}
    EnvironmentSensorMessageHandler(const EnvironmentSensorMessageHandler &) =
        delete;
    EnvironmentSensorMessageHandler(const EnvironmentSensorMessageHandler &&) =
        delete;
    auto operator=(const EnvironmentSensorMessageHandler &)
        -> EnvironmentSensorMessageHandler & = delete;
    auto operator=(const EnvironmentSensorMessageHandler &&)
        -> EnvironmentSensorMessageHandler && = delete;
    ~EnvironmentSensorMessageHandler() = default;

    // TODO(cm): we should move this to the env sensor driver when that's
    // complete
    void initialize() {
        std::array reg_buf{static_cast<uint8_t>(hdc2080::DEVICE_ID_REGISTER)};
        writer.transact(
            hdc2080::ADDRESS, reg_buf, 4, own_queue,
            utils::build_id(hdc2080::ADDRESS, hdc2080::DEVICE_ID_REGISTER, 0));
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        uint16_t configuration_data =
            hdc2080::DRDY_CONFIG << 8 | hdc2080::SAMPLE_RATE;
        writer.write(hdc2080::ADDRESS, configuration_data);
        configuration_data =
            hdc2080::INTERRUPT_REGISTER << 8 | hdc2080::SET_DATARDY;
        writer.write(hdc2080::ADDRESS, configuration_data);
        configuration_data = (hdc2080::MEASURE_REGISTER << 8) |
                             (hdc2080::BEGIN_MEASUREMENT_RECORDING);
        writer.write(hdc2080::ADDRESS, configuration_data);
        is_initialized = true;
    }

    void handle_message(const utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(const std::monostate &) {}

    void visit(const i2c::messages::TransactionResponse &m) {
        uint16_t data = 0x0;
        const auto *iter = m.read_buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, m.read_buffer.cend(), data);
        switch (utils::reg_from_id<uint8_t>(m.id.token)) {
            case hdc2080::LSB_HUMIDITY_REGISTER:
                LOG("Handling humidity data received %d", data);
                can_client.send_can_message(
                    can::ids::NodeId::host,
                    can::messages::ReadFromSensorResponse{
                        .sensor = can::ids::SensorType::humidity,
                        .sensor_id = sensor_id,
                        .sensor_data = hdc2080::convert(
                            data, can::ids::SensorType::humidity)});
                break;
            case hdc2080::LSB_TEMPERATURE_REGISTER:
                LOG("Handling temperature data recieved %d", data);
                can_client.send_can_message(
                    can::ids::NodeId::host,
                    can::messages::ReadFromSensorResponse{
                        .sensor = can::ids::SensorType::temperature,
                        .sensor_id = sensor_id,
                        .sensor_data = hdc2080::convert(
                            data, can::ids::SensorType::temperature)});
                break;
            default:
                // do nothing
                break;
        }
    }

    void visit(const can::messages::BaselineSensorRequest &) {
        LOG("Received non-supported BaselineSensorRequest");
    }

    void visit(const can::messages::SetSensorThresholdRequest &) {
        LOG("Received non-supported SetSensorThresholdRequest");
    }

    void visit(const can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        writer.write(hdc2080::ADDRESS, m.data);
    }

    void visit(const can::messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor", m.sensor);
        if (can::ids::SensorType(m.sensor) == can::ids::SensorType::humidity) {
            std::array reg_buf{hdc2080::LSB_HUMIDITY_REGISTER};
            writer.transact(hdc2080::ADDRESS, reg_buf, 2, own_queue,
                            utils::build_id(hdc2080::ADDRESS,
                                            hdc2080::LSB_HUMIDITY_REGISTER));
        } else {
            std::array reg_buf{hdc2080::LSB_TEMPERATURE_REGISTER};
            writer.transact(hdc2080::ADDRESS, reg_buf, 2, own_queue,
                            utils::build_id(hdc2080::ADDRESS,
                                            hdc2080::LSB_TEMPERATURE_REGISTER));
        }
    }

    void visit(const can::messages::BindSensorOutputRequest &) {
        LOG("Received non-supported BindSensorOutputRequest");
    }

    void visit(const can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device status request");
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PeripheralStatusResponse{
                .sensor = m.sensor,
                .sensor_id = sensor_id,
                .status = static_cast<uint8_t>(is_initialized)});
    }

    I2CQueueWriter &writer;
    CanClient &can_client;
    OwnQueue &own_queue;
    const can::ids::SensorId &sensor_id;
    bool is_initialized = false;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<utils::TaskMessage>, utils::TaskMessage>
class EnvironmentSensorTask {
  public:
    using Messages = utils::TaskMessage;
    using QueueType = QueueImpl<utils::TaskMessage>;
    EnvironmentSensorTask(QueueType &queue, can::ids::SensorId id)
        : queue{queue}, sensor_id{id} {}
    EnvironmentSensorTask(const EnvironmentSensorTask &c) = delete;
    EnvironmentSensorTask(const EnvironmentSensorTask &&c) = delete;
    auto operator=(const EnvironmentSensorTask &c) = delete;
    auto operator=(const EnvironmentSensorTask &&c) = delete;
    ~EnvironmentSensorTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 CanClient *can_client) {
        auto handler = EnvironmentSensorMessageHandler(*writer, *can_client,
                                                       get_queue(), sensor_id);
        //        handler.initialize();
        utils::TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
    can::ids::SensorId sensor_id;
};
};  // namespace tasks

};  // namespace sensors
