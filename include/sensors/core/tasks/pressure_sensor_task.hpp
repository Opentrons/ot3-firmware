#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/writer.hpp"
#include "pressure_driver.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

template <class I2CQueueWriter, class I2CQueuePoller,
          can::message_writer_task::TaskClient CanClient, class OwnQueue>
class PressureMessageHandler {
  public:
    explicit PressureMessageHandler(
        I2CQueueWriter &i2c_writer, I2CQueuePoller &i2c_poller,
        CanClient &can_client, OwnQueue &own_queue,
        sensors::hardware::SensorHardwareBase &hardware,
        const can::ids::SensorId &id)
        : driver{i2c_writer, i2c_poller, can_client, own_queue, hardware, id} {}
    PressureMessageHandler(const PressureMessageHandler &) = delete;
    PressureMessageHandler(const PressureMessageHandler &&) = delete;
    auto operator=(const PressureMessageHandler &)
        -> PressureMessageHandler & = delete;
    auto operator=(const PressureMessageHandler &&)
        -> PressureMessageHandler && = delete;
    ~PressureMessageHandler() = default;

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

    void visit(const i2c::messages::TransactionResponse &m) {
        driver.handle_response(m);
    }

    void visit(const can::messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (can::ids::SensorType(m.sensor) == can::ids::SensorType::pressure) {
            driver.get_pressure(mmr920C04::Registers::PRESSURE_READ);
        } else {
            driver.get_temperature();
        }
    }

    void visit(const can::messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        if (mmr920C04::is_valid_address(m.reg_address)) {
            driver.write(mmr920C04::Registers(m.reg_address), m.data);
        }
    }

    void visit(const can::messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        // poll a specific register, or default to a pressure read.
        if (can::ids::SensorType(m.sensor) == can::ids::SensorType::pressure) {
            driver.get_pressure(mmr920C04::Registers::PRESSURE_READ, true,
                                m.sample_rate);
        } else {
            driver.get_temperature(true, m.sample_rate);
        }
    }

    void visit(const can::messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor\n",
            m.threshold, m.sensor);
        driver.set_threshold(m.threshold);
        driver.send_threshold();
    }

    void visit(const can::messages::BindSensorOutputRequest &m) {
        LOG("received bind request but not implemented");
        static_cast<void>(m);
    }

    void visit(const can::messages::PeripheralStatusRequest &m) {
        LOG("received peripheral device info request");
        driver.send_peripheral_response();
        static_cast<void>(m);
    }

    MMR92C04<I2CQueueWriter, I2CQueuePoller, CanClient, OwnQueue> driver;
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
    PressureSensorTask(QueueType &queue) : queue{queue} {}
    PressureSensorTask(const PressureSensorTask &c) = delete;
    PressureSensorTask(const PressureSensorTask &&c) = delete;
    auto operator=(const PressureSensorTask &c) = delete;
    auto operator=(const PressureSensorTask &&c) = delete;
    ~PressureSensorTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        i2c::writer::Writer<QueueImpl> *writer,
        i2c::poller::Poller<QueueImpl> *poller, CanClient *can_client,
        sensors::hardware::SensorHardwareBase *hardware,
        const can::ids::SensorId *id) {
        auto handler = PressureMessageHandler{
            *writer, *poller, *can_client, get_queue(), *hardware, *id};
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
