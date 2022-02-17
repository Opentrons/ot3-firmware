#pragma once

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "sensors/core/utils.hpp"

namespace sensor_message_handler {

template <sensor_task_utils::TaskClient SensorTaskClient>
class SensorHandler {
  public:
    explicit SensorHandler(SensorTaskClient &client) : client(client) {}
    SensorHandler(const SensorHandler &) = delete;
    SensorHandler(const SensorHandler &&) = delete;
    auto operator=(const SensorHandler &) -> SensorHandler & = delete;
    auto operator=(const SensorHandler &&) -> SensorHandler && = delete;
    ~SensorHandler() = default;

    void handle(sensor_task_utils::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    SensorTaskClient &client;

    void visit(std::monostate &m) {}

    void visit(can_messages::SetSensorThresholdRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(can_messages::BaselineSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(can_messages::WriteToSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(can_messages::ReadFromSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void send_to_queue(can_ids::SensorType type,
                       sensor_task_utils::TaskMessage m) {
        switch (type) {
            case can_ids::SensorType::temperature: {
                client.send_environment_sensor_queue(m);
                break;
            }
            case can_ids::SensorType::humidity: {
                client.send_environment_sensor_queue(m);
            }
            default:
                break;
        }
    }
};

}  // namespace sensor_message_handler