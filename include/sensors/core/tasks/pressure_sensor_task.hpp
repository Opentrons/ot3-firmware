#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "sensors/core/mmr920C04.hpp"
#include "sensors/core/utils.hpp"

namespace pressure_sensor_task {

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class PressureMessageHandler {
  public:
    explicit PressureMessageHandler(I2CQueueWriter &i2c_writer,
                                    CanClient &can_client)
        : driver{i2c_writer, can_client}, can_client{can_client} {}
    PressureMessageHandler(const PressureMessageHandler &) = delete;
    PressureMessageHandler(const PressureMessageHandler &&) = delete;
    auto operator=(const PressureMessageHandler &)
        -> PressureMessageHandler & = delete;
    auto operator=(const PressureMessageHandler &&)
        -> PressureMessageHandler && = delete;
    ~PressureMessageHandler() = default;

    void handle_message(sensor_task_utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void initialize() {
        driver.write_config();
        if (driver.initialized()) {
            LOG("Driver initialized successfully")
        } else {
            LOG("Driver not initialized successfully")
        }
    }

  private:
    void visit(std::monostate &m) {}

    void visit(can_messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (m.reg_address &
            mmr920C04_registers::is_valid_address(m.reg_address)) {
            driver.write(mmr920C04_registers::Registers(m.reg_address), 0x0);
            driver.read(mmr920C04_registers::Registers(m.reg_address));
        } else {
            driver.write(mmr920C04_registers::Registers::PRESSURE_READ, 0x0);
            driver.read(mmr920C04_registers::Registers::PRESSURE_READ);
        }
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        if (mmr920C04_registers::is_valid_address(m.reg_address)) {
            driver.write(mmr920C04_registers::Registers(m.reg_address), m.data);
        }
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        // poll a specific register, or default to a pressure read.
        if (mmr920C04_registers::is_valid_address(m.reg_address)) {
            driver.poll_read(m.reg_address, m.sample_rate, DELAY);
        } else {
            driver.poll_read(mmr920C04_registers::Registers::PRESSURE_READ,
                             m.sample_rate, DELAY);
        }
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor\n",
            m.threshold, m.sensor);
        driver.set_threshold(m.threshold);
        auto message = can_messages::SensorThresholdResponse{
            {}, driver.get_sensor_id(), driver.get_threshold()};
        can_client.send_can_message(driver.get_host_id(), message);
    }

    uint16_t DELAY = 20;
    mmr920C04::MMR92C04<I2CQueueWriter, CanClient> driver;
    CanClient &can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, class I2CQueueWriter,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<sensor_task_utils::TaskMessage>,
                      sensor_task_utils::TaskMessage>
class PressureSensorTask {
  public:
    using QueueType = QueueImpl<sensor_task_utils::TaskMessage>;
    PressureSensorTask(QueueType &queue) : queue{queue} {}
    PressureSensorTask(const PressureSensorTask &c) = delete;
    PressureSensorTask(const PressureSensorTask &&c) = delete;
    auto operator=(const PressureSensorTask &c) = delete;
    auto operator=(const PressureSensorTask &&c) = delete;
    ~PressureSensorTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(I2CQueueWriter *writer,
                                 CanClient *can_client) {
        auto handler = PressureMessageHandler{*writer, *can_client};
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

}  // namespace pressure_sensor_task