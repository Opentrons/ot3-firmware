#pragma once

#include <array>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/core/tasks/capacitive_sensor_callbacks.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

using namespace hardware;

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class CapacitiveMessageHandler {
  public:
    explicit CapacitiveMessageHandler(I2CQueueWriter &i2c_writer,
                                      I2CQueuePoller &i2c_poller,
                                      SensorHardwareBase &hardware,
                                      CanClient &can_client,
                                      OwnQueue &own_queue,
                                      can::ids::SensorId &id)
        : writer{i2c_writer},
          poller{i2c_poller},
          hardware{hardware},
          can_client{can_client},
          own_queue{own_queue},
          sensor_id{id},
          capacitance_handler{can_client, writer, hardware, sensor_id} {}
    CapacitiveMessageHandler(const CapacitiveMessageHandler &) = delete;
    CapacitiveMessageHandler(const CapacitiveMessageHandler &&) = delete;
    auto operator=(const CapacitiveMessageHandler &)
        -> CapacitiveMessageHandler & = delete;
    auto operator=(const CapacitiveMessageHandler &&)
        -> CapacitiveMessageHandler && = delete;
    ~CapacitiveMessageHandler() = default;

    void handle_message(utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    // TODO(cm): we should move this to the capacitive sensor driver class when
    // that is complete
    void initialize() {
        std::array reg_buf{static_cast<uint8_t>(DEVICE_ID_REGISTER)};
        writer.transact(ADDRESS, reg_buf, 2, own_queue,
                        utils::build_id(ADDRESS, DEVICE_ID_REGISTER, 0));
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        capacitance_handler.initialize();
        is_initialized = true;
    }

  private:
    void visit(std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        if (utils::reg_from_id<uint8_t>(m.id.token) != MSB_MEASUREMENT_1) {
            return;
        }
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::POLL_IS_CONTINUOUS)) {
            capacitance_handler.handle_ongoing_response(m);
            LOG("continuous transaction response");
        } else {
            capacitance_handler.handle_baseline_response(m);
            LOG("limited transaction response");
        }
    }

    void visit(can::messages::ReadFromSensorRequest &m) {
        /**
         * The FDC1004 sensor has an offset register and
         * a measurement register. The CAPDAC offset is used
         * to account for any additional baseline noise that
         * may occur on the capacitve sensor.
         *
         */
        LOG("Received request to read from %d sensor", m.sensor);
        if (bool(m.offset_reading)) {
            auto message = can::messages::ReadFromSensorResponse{
                .message_index = m.message_index,
                .sensor = SensorType::capacitive,
                .sensor_id = sensor_id,
                .sensor_data = convert_to_fixed_point(
                    capacitance_handler.get_offset(), S15Q16_RADIX)};
            can_client.send_can_message(can::ids::NodeId::host, message);
        } else {
            capacitance_handler.reset_limited();
            capacitance_handler.set_number_of_reads(1);
            poller.multi_register_poll(
                ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, 1, DELAY,
                own_queue, utils::build_id(ADDRESS, MSB_MEASUREMENT_1, 1));
        }
    }

    void visit(can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        writer.write(ADDRESS, m.data);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(can::messages::BaselineSensorRequest &m) {
        LOG("Received request to baseline %d sensor", m.sensor);
        capacitance_handler.reset_limited();
        capacitance_handler.set_number_of_reads(m.sample_rate);
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::IS_BASELINE};
        poller.multi_register_poll(
            ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, m.sample_rate,
            DELAY, own_queue,
            utils::build_id(ADDRESS, MSB_MEASUREMENT_1,
                            utils::byte_from_tags(tags)));
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(can::messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor",
            m.threshold, m.sensor);
        if (m.mode == can::ids::SensorThresholdMode::absolute) {
            capacitance_handler.set_threshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX), m.mode,
                m.message_index);
        } else {
            capacitance_handler.reset_limited();
            capacitance_handler.set_number_of_reads(10);
            std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                            utils::ResponseTag::IS_BASELINE,
                            utils::ResponseTag::IS_THRESHOLD_SENSE};
            capacitance_handler.prime_autothreshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX));
            poller.multi_register_poll(
                ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2,
                uint16_t(10), DELAY, own_queue,
                utils::build_id(ADDRESS, MSB_MEASUREMENT_1,
                                utils::byte_from_tags(tags)));
        }
    }

    inline auto delay_or_disable(uint8_t binding) -> uint8_t {
        if (binding ==
            static_cast<uint8_t>(can::ids::SensorOutputBinding::none)) {
            return 0;
        }
        return DELAY;
    }

    void visit(can::messages::BindSensorOutputRequest &m) {
        capacitance_handler.set_echoing(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::report));
        capacitance_handler.set_bind_sync(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::sync));
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::POLL_IS_CONTINUOUS};
        const auto delay = delay_or_disable(m.binding);
        poller.continuous_multi_register_poll(
            ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, delay,
            own_queue,
            utils::build_id(ADDRESS, MSB_MEASUREMENT_1,
                            utils::byte_from_tags(tags)));
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device status request");
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PeripheralStatusResponse{
                .message_index = m.message_index,
                .sensor = m.sensor,
                .sensor_id = sensor_id,
                .status = static_cast<uint8_t>(is_initialized)});
    }

    utils::BitMode mode = utils::BitMode::MSB;
    static constexpr uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    SensorHardwareBase &hardware;
    CanClient &can_client;
    OwnQueue &own_queue;
    can::ids::SensorId &sensor_id;
    bool is_initialized = false;

  public:
    // Kept public for testability
    ReadCapacitanceCallback<CanClient, I2CQueueWriter> capacitance_handler;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<utils::TaskMessage>, utils::TaskMessage>
class CapacitiveSensorTask {
  public:
    using Messages = utils::TaskMessage;
    using QueueType = QueueImpl<utils::TaskMessage>;
    CapacitiveSensorTask(QueueType &queue, can::ids::SensorId id)
        : queue{queue}, sensor_id{id} {}
    CapacitiveSensorTask(const CapacitiveSensorTask &c) = delete;
    CapacitiveSensorTask(const CapacitiveSensorTask &&c) = delete;
    auto operator=(const CapacitiveSensorTask &c) = delete;
    auto operator=(const CapacitiveSensorTask &&c) = delete;
    ~CapacitiveSensorTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 i2c::poller::Poller<QueueImpl> *poller,
                                 SensorHardwareBase *hardware,
                                 CanClient *can_client) {
        auto handler = CapacitiveMessageHandler{
            *writer, *poller, *hardware, *can_client, get_queue(), sensor_id};
        handler.initialize();
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
}  // namespace tasks
}  // namespace sensors
