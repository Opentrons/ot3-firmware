#pragma once

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace handlers {

template <utils::TaskClient SensorTaskClient>
class SensorHandler {
  public:
    explicit SensorHandler(SensorTaskClient &client) : client(client) {}
    SensorHandler(const SensorHandler &) = delete;
    SensorHandler(const SensorHandler &&) = delete;
    auto operator=(const SensorHandler &) -> SensorHandler & = delete;
    auto operator=(const SensorHandler &&) -> SensorHandler && = delete;
    ~SensorHandler() = default;

    void handle(const utils::CanMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    SensorTaskClient &client;

    void visit(const std::monostate &) {}

    void visit(const can_messages::SetSensorThresholdRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(const can_messages::BaselineSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(const can_messages::WriteToSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(const can_messages::ReadFromSensorRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(const can_messages::BindSensorOutputRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m);
    }

    void visit(const can_messages::PeripheralStatusRequest &m) {
        send_to_queue(can_ids::SensorType(m.sensor), m)
    };

    void send_to_queue(can_ids::SensorType type, const utils::TaskMessage &m) {
        switch (type) {
            case can_ids::SensorType::temperature: {
                client.send_environment_sensor_queue(m);
                break;
            }
            case can_ids::SensorType::humidity: {
                client.send_environment_sensor_queue(m);
                break;
            }
            case can_ids::SensorType::capacitive: {
                client.send_capacitive_sensor_queue(m);
                break;
            }
            case can_ids::SensorType::pressure: {
                client.send_pressure_sensor_queue(m);
                break;
            }
            default:
                break;
        }
    }
};
};  // namespace handlers
};  // namespace sensors
