#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"
#include "sensors/core/tasks/pressure_sensor_callbacks.hpp"
#include "sensors/core/utils.hpp"

namespace pressure_sensor_task {

using namespace pressure_sensor_callbacks;

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class PressureMessageHandler {
  public:
    explicit PressureMessageHandler(I2CQueueWriter &i2c_writer,
                                    CanClient &can_client)
        : writer{i2c_writer},
          can_client{can_client},
          capacitance_callback{can_client, CAPDAC_OFFSET},
          capdac_callback{can_client, CAPDAC_OFFSET} {}
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
        writer.write(RESET_REGISTER, ADDRESS);
        vTaskDelay(10);
        writer.write(ACTIVE_MEASUREMENT_M4, ADDRESS);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(can_messages::ReadFromSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
        if (bool(m.offset_reading)) {

        } else {

        }
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        LOG("Received request to write data %d to %d sensor\n", m.data,
            m.sensor);
        writer.write(m.data, ADDRESS);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        LOG("Received request to read from %d sensor\n", m.sensor);
    }

    void visit(can_messages::SetSensorThresholdRequest &m) {
        LOG("Received request to set threshold to %d from %d sensor\n",
            m.threshold, m.sensor);
        THRESHOLD = m.threshold;
        auto message = can_messages::SensorThresholdResponse{
            {}, SensorType::pressure, THRESHOLD};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    InternalCallback internal_callback{};
    uint32_t THRESHOLD = 0x8;
    uint16_t DELAY = 20;
    I2CQueueWriter &writer;
    CanClient &can_client;
    ReadPressureCallback<CanClient> pressure_handler;
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