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
          message_writer_task::TaskClient CanClient, class OwnQueue>
class CapacitiveMessageHandler {
  public:
    explicit CapacitiveMessageHandler(I2CQueueWriter &i2c_writer,
                                      I2CQueuePoller &i2c_poller,
                                      SensorHardwareBase &hardware,
                                      CanClient &can_client,
                                      OwnQueue &own_queue)
        : writer{i2c_writer},
          poller{i2c_poller},
          hardware{hardware},
          can_client{can_client},
          own_queue{own_queue},
          capacitance_handler{can_client, writer, hardware, zero_threshold,
                              capdac_offset} {}
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

    void initialize() {
        std::array reg_buf{static_cast<uint8_t>(DEVICE_ID_REGISTER)};
        writer.transact(ADDRESS, reg_buf, 4, own_queue,
                        utils::build_id(ADDRESS, DEVICE_ID_REGISTER, false));
        // We should send a message that the sensor is in a ready state,
        // not sure if we should have a separate can message to do that
        // holding off for this PR.
        uint32_t configuration_data =
            CONFIGURATION_MEASUREMENT << 8 | DEVICE_CONFIGURATION;
        writer.write(ADDRESS, configuration_data);
        configuration_data = FDC_CONFIGURATION << 8 | SAMPLE_RATE;
        writer.write(ADDRESS, configuration_data);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(i2c::messages::TransactionResponse &m) {
        if (utils::reg_from_id<uint8_t>(m.id.token) != MSB_MEASUREMENT_1) {
            return;
        }
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::POLL_IS_CONTINUOUS)) {
            capacitance_handler.handle_ongoing_response(m);
        } else {
            capacitance_handler.handle_baseline_response(m);
        }
    }

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
            capacitance_handler.reset_limited();
            capacitance_handler.set_number_of_reads(1);
            poller.multi_register_poll(
                ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, 1, DELAY,
                own_queue, utils::build_id(ADDRESS, MSB_MEASUREMENT_1, true));
        }
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        writer.write(ADDRESS, m.data);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor", m.sensor);
        capdac_offset = capacitance_handler.get_offset();
        capacitance_handler.reset_limited();
        capacitance_handler.set_number_of_reads(m.sample_rate);
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::IS_BASELINE};
        poller.multi_register_poll(
            ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, m.sample_rate,
            DELAY, own_queue,
            utils::build_id(ADDRESS, MSB_MEASUREMENT_1,
                            utils::byte_from_tags(tags)));
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor",
            m.threshold, m.sensor);
        zero_threshold = m.threshold;

        auto message = can_messages::SensorThresholdResponse{
            .sensor = SensorType::capacitive, .threshold = zero_threshold};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void visit(can_messages::BindSensorOutputRequest &m) {
        capacitance_handler.set_echoing(
            m.binding &
            static_cast<uint8_t>(can_ids::SensorOutputBinding::report));
        capacitance_handler.set_echoing(
            m.binding &
            static_cast<uint8_t>(can_ids::SensorOutputBinding::sync));
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::POLL_IS_CONTINUOUS};
        poller.continuous_multi_register_poll(
            ADDRESS, MSB_MEASUREMENT_1, 2, LSB_MEASUREMENT_1, 2, DELAY,
            own_queue,
            utils::build_id(ADDRESS, MSB_MEASUREMENT_1,
                            utils::byte_from_tags(tags)));
    }

    utils::BitMode mode = utils::BitMode::MSB;
    // 3 pF
    int32_t zero_threshold = 0x3;
    // 0 pF
    float capdac_offset = 0x0;
    static constexpr uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    I2CQueuePoller &poller;
    SensorHardwareBase &hardware;
    CanClient &can_client;
    OwnQueue &own_queue;
    ReadCapacitanceCallback<CanClient, I2CQueueWriter> capacitance_handler;
    uint32_t sensor_bindings = 0;
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
    CapacitiveSensorTask(QueueType &queue) : queue{queue} {}
    CapacitiveSensorTask(const CapacitiveSensorTask &c) = delete;
    CapacitiveSensorTask(const CapacitiveSensorTask &&c) = delete;
    auto operator=(const CapacitiveSensorTask &c) = delete;
    auto operator=(const CapacitiveSensorTask &&c) = delete;
    ~CapacitiveSensorTask() = default;

    /**
     * Task entry point.
     */
    template <message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 i2c::poller::Poller<QueueImpl> *poller,
                                 SensorHardwareBase *hardware,
                                 CanClient *can_client) {
        auto handler = CapacitiveMessageHandler{*writer, *poller, *hardware,
                                                *can_client, get_queue()};
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
};
}  // namespace tasks
}  // namespace sensors
