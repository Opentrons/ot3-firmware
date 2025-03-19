#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "sensors/core/mmr920.hpp"
#include "sensors/core/tasks/pressure_driver.hpp"
#include "sensors/core/utils.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"

namespace sensors {
namespace tasks {

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue,
          usage_storage_task::TaskClient UsageClient>
class PressureMessageHandler {
  public:
    explicit PressureMessageHandler(
        I2CQueueWriter &i2c_writer, I2CQueuePoller &i2c_poller,
        CanClient &can_client, OwnQueue &own_queue,
        sensors::hardware::SensorHardwareBase &hardware,
        const can::ids::SensorId &id,
        std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer,
        UsageClient& usage_client, uint16_t pres_err_key)
        : driver{i2c_writer, i2c_poller, can_client,   own_queue,
                 hardware,   id,         sensor_buffer, usage_client, pres_err_key},
          sensor_id{id} {}
    PressureMessageHandler(const PressureMessageHandler &) = delete;
    PressureMessageHandler(const PressureMessageHandler &&) = delete;
    auto operator=(const PressureMessageHandler &)
        -> PressureMessageHandler & = delete;
    auto operator=(const PressureMessageHandler &&)
        -> PressureMessageHandler && = delete;
    ~PressureMessageHandler() { driver.reset_readings(); }

    void handle_message(const utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void initialize() {
        driver.write_config();
        if (driver.initialized()) {
            LOG("Driver initialized successfully");
        } else {
            LOG("Driver not initialized successfully");
        }
    }

  private:
    void visit(const std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        auto reg_id = utils::reg_from_id<mmr920::Registers>(m.id.token);
        if ((reg_id != mmr920::Registers::LOW_PASS_PRESSURE_READ) &&
            (reg_id != mmr920::Registers::PRESSURE_READ) &&
            (reg_id != mmr920::Registers::TEMPERATURE_READ) &&
            (reg_id != mmr920::Registers::STATUS)) {
            return;
        }
        // may not be routed to baseline function like we suspect
        if (utils::tag_in_token(m.id.token,
                                utils::ResponseTag::POLL_IS_CONTINUOUS)) {
            if (reg_id == mmr920::Registers::TEMPERATURE_READ) {
                driver.handle_ongoing_temperature_response(m);
            } else {
                driver.handle_ongoing_pressure_response(m);
            }
            LOG("continuous transaction response");
        } else {
            if (reg_id == mmr920::Registers::TEMPERATURE_READ) {
                driver.handle_baseline_temperature_response(m);
            } else {
                driver.handle_baseline_pressure_response(m);
            }

            LOG("limited transaction response");
        }
    }

    void visit(const can::messages::SendAccumulatedSensorDataRequest &m) {
        LOG("Received request to dump pressure data buffer");
        driver.send_accumulated_sensor_data(m.message_index);
    }

    void visit(const can::messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        driver.set_echoing(true);
        if (can::ids::SensorType(m.sensor) == can::ids::SensorType::pressure) {
            auto tags_as_int = 1;
            driver.poll_limited_pressure(1, tags_as_int);
        }
        if (can::ids::SensorType(m.sensor) ==
            can::ids::SensorType::pressure_temperature) {
            auto tags_as_int = 1;
            driver.poll_limited_temperature(1, tags_as_int);
        }
    }

    void visit(const can::messages::MaxSensorValueRequest &m) {
        auto max = mmr920::get_max_pressure_reading(driver.sensor_version());
        auto message = can::messages::ReadFromSensorResponse{
            .message_index = m.message_index,
            .sensor = SensorType::pressure,
            .sensor_id = sensor_id,
            .sensor_data = mmr920::reading_to_fixed_point(max)};
        driver.get_can_client().send_can_message(can::ids::NodeId::host,
                                                 message);
    }

    void visit(const can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        if (mmr920::is_valid_address(m.reg_address)) {
            driver.write(mmr920::Registers(m.reg_address), m.data);
        }
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor",
            m.threshold, m.sensor);
        // NOTE this function only supports pressure reads right now.
        driver.set_echoing(true);
        if (m.mode == can::ids::SensorThresholdMode::absolute) {
            driver.set_threshold(
                fixed_point_to_float(m.threshold, S15Q16_RADIX), m.mode,
                m.message_index);
        } else {
            std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                            utils::ResponseTag::IS_BASELINE,
                            utils::ResponseTag::IS_THRESHOLD_SENSE};
            auto tags_as_int = utils::byte_from_tags(tags);
            driver.poll_limited_pressure(10, tags_as_int);
        }
    }

    void visit(const can::messages::BindSensorOutputRequest &m) {
        driver.set_echoing(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::report));
        driver.set_multi_sensor_sync(
            m.binding & static_cast<uint8_t>(
                            can::ids::SensorOutputBinding::multi_sensor_sync));
        driver.set_bind_sync(
            m.binding &
            static_cast<uint8_t>(can::ids::SensorOutputBinding::sync));
        driver.set_max_bind_sync(
            m.binding & static_cast<uint8_t>(
                            can::ids::SensorOutputBinding::max_threshold_sync));
        driver.set_auto_baseline_report(
            m.binding &
            static_cast<uint8_t>(
                can::ids::SensorOutputBinding::auto_baseline_report));
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::POLL_IS_CONTINUOUS};
        auto tags_as_int = utils::byte_from_tags(tags);

        if (can::ids::SensorType(m.sensor) ==
            can::ids::SensorType::pressure_temperature) {
            driver.poll_continuous_temperature(tags_as_int);
        } else {
            driver.poll_continuous_pressure(tags_as_int);
        }
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::BaselineSensorRequest &m) {
        LOG("received baseline request");
        static_cast<void>(m);
        std::array tags{utils::ResponseTag::IS_PART_OF_POLL,
                        utils::ResponseTag::IS_BASELINE,
                        utils::ResponseTag::IS_THRESHOLD_SENSE};
        auto tags_as_int = utils::byte_from_tags(tags);
        driver.set_echoing(true);
        if (can::ids::SensorType(m.sensor) ==
            can::ids::SensorType::pressure_temperature) {
            driver.poll_limited_temperature(m.number_of_reads, tags_as_int);
        } else {
            driver.poll_limited_pressure(m.number_of_reads, tags_as_int);
        }
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device info request");
        driver.send_peripheral_response(m.message_index);
        static_cast<void>(m);
    }

    MMR920<I2CQueueWriter, I2CQueuePoller, CanClient, OwnQueue, UsageClient> driver;
    can::ids::SensorId sensor_id;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<utils::TaskMessage>, utils::TaskMessage>
class PressureSensorTask {
  public:
    using Messages = utils::TaskMessage;
    using QueueType = QueueImpl<utils::TaskMessage>;
    PressureSensorTask(QueueType &queue, can::ids::SensorId id)
        : queue{queue}, sensor_id{id} {}
    PressureSensorTask(const PressureSensorTask &c) = delete;
    PressureSensorTask(const PressureSensorTask &&c) = delete;
    auto operator=(const PressureSensorTask &c) = delete;
    auto operator=(const PressureSensorTask &&c) = delete;
    ~PressureSensorTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient,
          usage_storage_task::TaskClient UsageClient>
    [[noreturn]] void operator()(
        i2c::writer::Writer<QueueImpl> *writer,
        i2c::poller::Poller<QueueImpl> *poller, CanClient *can_client,
        sensors::hardware::SensorHardwareBase *hardware,
        std::array<float, SENSOR_BUFFER_SIZE> *sensor_buffer,
        UsageClient *usage_client, uint16_t pres_err_key) {
        auto handler = PressureMessageHandler{
            *writer,   *poller,   *can_client,  get_queue(),
            *hardware, sensor_id, sensor_buffer, *usage_client, pres_err_key};
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
};  // namespace tasks
};  // namespace sensors
