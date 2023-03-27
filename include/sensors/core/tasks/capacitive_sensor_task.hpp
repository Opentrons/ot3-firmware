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
#include "sensors/core/tasks/capacitive_driver.hpp"
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
                                      bool shared_task)
        : driver{i2c_writer, i2c_poller, can_client, own_queue, hardware, shared_task} {}
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

    void initialize(can::ids::SensorId _id) {
        driver.initialize(_id);
    }

  private:
    void visit(std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        auto reg_id = utils::reg_from_id<uint8_t>(m.id.token);
        if (reg_id != fdc1004::Registers::MEAS1_MSB | reg_id != fdc1004::Registers::MEAS2_MSB) {
            return;
        }
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::POLL_IS_CONTINUOUS)) {
            driver.handle_ongoing_response(m);
            LOG("continuous transaction response");
        } else {
            driver.handle_baseline_response(m);
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
                .sensor_id = m.sensor_id,
                .sensor_data = convert_to_fixed_point(
                    driver.get_offset(), S15Q16_RADIX)};
            driver.get_can_client().send_can_message(can::ids::NodeId::host, message);
        } else {
            driver.reset_limited();
            driver.set_number_of_reads(1);
            std::array tags{};
            driver.poll_limited_capacitance(1, m.sensor_id, tags);
        }
    }

    void visit(can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        if (fdc1004::is_valid_address(m.reg_address)) {
            driver.write(fdc1004::Registers(m.reg_address), m.data);
        }
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(can::messages::BaselineSensorRequest &m) {
        LOG("Received request to baseline %d sensor", m.sensor);
        // FIXME remove the baseline sensor request message
        // in favor of using the auto-baseline mode.
        // We should also enable this for the other drivers as
        // well. Where possible we want to keep driver behavior
        // consistent.
        driver.set_sync_bind(can::ids::SensorOutputBinding::none);
        driver.reset_limited();
        driver.set_number_of_reads(m.number_of_reads);

        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::IS_BASELINE};
        driver.poll_limited_capacitance(m.number_of_reads, m.sensor_id, tags);
        driver.get_can_client().send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(can::messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor",
            m.threshold, m.sensor);
        if (m.mode == can::ids::SensorThresholdMode::absolute) {
            driver.set_threshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX), m.mode,
                m.message_index);
        } else {
            driver.reset_limited();
            driver.set_number_of_reads(10);
            std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                            utils::ResponseTag::IS_BASELINE,
                            utils::ResponseTag::IS_THRESHOLD_SENSE};
            driver.prime_autothreshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX));
            driver.poll_limited_capacitance(10, m.sensor_id, tags);
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
        driver.set_echoing(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::report));
        driver.set_bind_sync(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::sync));
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::POLL_IS_CONTINUOUS};
        const auto delay = delay_or_disable(m.binding);
        driver.poll_continuous_capacitance(m.sensor_id, tags);
        driver.get_can_client().send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device status request");
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host,
            can::messages::PeripheralStatusResponse{
                .message_index = m.message_index,
                .sensor = m.sensor,
                .sensor_id = m.sensor_id,
                .status = static_cast<uint8_t>(driver.initialized())});
    }

    FDC1004<I2CQueueWriter, I2CQueuePoller, CanClient, OwnQueue> driver;
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
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 i2c::poller::Poller<QueueImpl> *poller,
                                 SensorHardwareBase *hardware,
                                 CanClient *can_client,
                                 can::ids::SensorId _id = can::ids::SensorId::S0,
                                 bool shared_task = false
                                 ) {
        // On the 8 channel, there is a singular cap sensor but we're using
        // multiple channels. We will thus rely on the sensor id in this case
        // to determine which CIN configuration the sensor should be in.
        auto handler = CapacitiveMessageHandler{
            *writer, *poller, *hardware, *can_client, get_queue(), shared_task};
        handler.initialize(_id);
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
