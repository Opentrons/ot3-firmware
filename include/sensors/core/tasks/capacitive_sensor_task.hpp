#pragma once

#include <array>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/tasks/capacitive_driver.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class CapacitiveMessageHandler {
  public:
    explicit CapacitiveMessageHandler(
        I2CQueueWriter &i2c_writer, I2CQueuePoller &i2c_poller,
        sensors::hardware::SensorHardwareBase &hardware, CanClient &can_client,
        OwnQueue &own_queue, bool shared_task,
        std::array<float, SENSOR_BUFFER_SIZE> *p_buff)
        : driver{i2c_writer, i2c_poller,  can_client, own_queue,
                 hardware,   shared_task, p_buff} {}
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

    void initialize() { driver.initialize(); }

  private:
    void visit(std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        auto reg_id = utils::reg_from_id<uint8_t>(m.id.token);
        if (reg_id == static_cast<uint8_t>(fdc1004::Registers::FDC_CONF)) {
            driver.handle_fdc_response(m);
            return;
        }
        if ((reg_id != static_cast<uint8_t>(fdc1004::Registers::MEAS1_MSB)) &&
            (reg_id != static_cast<uint8_t>(fdc1004::Registers::MEAS2_MSB)) &&
            (reg_id != static_cast<uint8_t>(fdc1004::Registers::MEAS1_LSB)) &&
            (reg_id != static_cast<uint8_t>(fdc1004::Registers::MEAS2_LSB))) {
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

    void visit(const can::messages::SendAccumulatedSensorDataRequest &m) {
        LOG("Received request to dump capacitive data buffer %d", m.sensor_id);

        driver.send_accumulated_sensor_data(m.message_index);
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
                .sensor = can::ids::SensorType::capacitive,
                .sensor_id = static_cast<can::ids::SensorId>(m.sensor_id),
                .sensor_data =
                    convert_to_fixed_point(driver.get_offset(), S15Q16_RADIX)};
            driver.get_can_client().send_can_message(can::ids::NodeId::host,
                                                     message);
        } else {
            driver.reset_limited();
            driver.set_number_of_reads(1);
            auto tags_as_int = 1;
            driver.poll_limited_capacitance(
                1, static_cast<can::ids::SensorId>(m.sensor_id), tags_as_int);
        }
    }

    void visit(can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor", m.data, m.sensor);
        // FIXME we should send a response message after a write request
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
        driver.reset_limited();
        driver.set_number_of_reads(m.number_of_reads);

        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::IS_BASELINE};
        auto tags_as_int = utils::byte_from_tags(tags);
        driver.poll_limited_capacitance(
            m.number_of_reads, static_cast<can::ids::SensorId>(m.sensor_id),
            tags_as_int);
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
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
            auto tags_as_int = utils::byte_from_tags(tags);
            driver.prime_autothreshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX));
            driver.poll_limited_capacitance(
                10, static_cast<can::ids::SensorId>(m.sensor_id), tags_as_int);
        }
    }

    void visit(can::messages::BindSensorOutputRequest &m) {
        driver.set_echoing(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::report));
        driver.set_bind_sync(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::sync));
        driver.set_max_bind_sync(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::sync));
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::POLL_IS_CONTINUOUS};
        auto tags_as_int = utils::byte_from_tags(tags);
        driver.poll_continuous_capacitance(
            static_cast<can::ids::SensorId>(m.sensor_id), tags_as_int,
            m.binding);
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
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

    // FIXME now that we have a separate driver class, we should be
    // able to separately test message handling and driver functionality
    // rather than relying on this variable being public
  public:
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
    [[noreturn]] void operator()(
        i2c::writer::Writer<QueueImpl> *writer,
        i2c::poller::Poller<QueueImpl> *poller,
        sensors::hardware::SensorHardwareBase *hardware, CanClient *can_client,
        std::array<float, SENSOR_BUFFER_SIZE> *p_buff,
        bool shared_task = false) {
        // On the 8 channel, there is a singular cap sensor but we're using
        // multiple channels. We will thus rely on the sensor id in this case
        // to determine which CIN configuration the sensor should be in.
        auto handler = CapacitiveMessageHandler{
            *writer,     *poller,     *hardware, *can_client,
            get_queue(), shared_task, p_buff};
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
