#pragma once

#include <array>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/poller.hpp"
#include "sensors/core/tasks/environment_driver.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class EnvironmentSensorMessageHandler {
  public:
    EnvironmentSensorMessageHandler(I2CQueueWriter &i2c_writer,
                                    I2CQueuePoller &i2c_poller,
                                    CanClient &can_client, OwnQueue &own_queue,
                                    const can::ids::SensorId &id)
        : driver{i2c_writer, i2c_poller, can_client, own_queue, id} {}
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
        /*
         * (lc 7-26-2022) Not sure if a delay is needed
         * but the sensor can only start to send values
         * after 3ms of boot time.
         */
        driver.initialize();
    }

    void handle_message(const utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(const std::monostate &) {}

    void visit(const i2c::messages::TransactionResponse &m) {
        driver.handle_response(m);
    }

    void visit(const can::messages::BaselineSensorRequest &m) {
        driver.trigger_on_demand(m.number_of_reads);
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::SetSensorThresholdRequest &) {
        LOG("Received non-supported SetSensorThresholdRequest");
    }

    void visit(const can::messages::WriteToSensorRequest &m) {
        LOG("Received non-supported WriteToSensorRequest");
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::ReadFromSensorRequest &) {
        LOG("Received request to read from environment sensor");
        driver.trigger_on_demand();
    }

    void visit(const can::messages::SendAccumulatedPressureDataRequest &) {
        LOG("Received request to dump pressure data buffer %d", sensor_id);
    }

    void visit(const can::messages::BindSensorOutputRequest &m) {
        LOG("Received bind sensor output request from %d sensor", m.sensor);
        // sync doesn't quite mean the same thing here for us. We should
        // think about potentially creating a separate CAN message
        // for the hdc sensor to at least set the power mode and
        // auto measure frequency.
        driver.set_bind_flags(m.binding);
        if (m.binding ==
            static_cast<uint8_t>(can::ids::SensorOutputBinding::none)) {
            driver.auto_measure_mode(hdc3020::Registers::RESET);
        } else {
            driver.auto_measure_mode(hdc3020::Registers::AUTO_MEASURE_1M1S);
        }
        driver.get_can_client().send_can_message(
            can::ids::NodeId::host, can::messages::ack_from_request(m));
    }

    void visit(const can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device status request");
        driver.send_status(m);
    }

    HDC3020<I2CQueueWriter, I2CQueuePoller, CanClient, OwnQueue> driver;
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
                                 i2c::poller::Poller<QueueImpl> *poller,
                                 CanClient *can_client) {
        auto handler = EnvironmentSensorMessageHandler(
            *writer, *poller, *can_client, get_queue(), sensor_id);
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
