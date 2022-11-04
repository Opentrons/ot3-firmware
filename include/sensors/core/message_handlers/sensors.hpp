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

    void visit(const can::messages::SetSensorThresholdRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
    }

    void visit(const can::messages::BaselineSensorRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
    }

    void visit(const can::messages::WriteToSensorRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
        can_client.send_can_message(can::ids::NodeId::host,
            can::messages::Acknowledgment{.message_index = m.message_index});
    }

    void visit(const can::messages::ReadFromSensorRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
    }

    void visit(const can::messages::BindSensorOutputRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
    }

    void visit(const can::messages::PeripheralStatusRequest &m) {
        send_to_queue(can::ids::SensorType(m.sensor),
                      can::ids::SensorId(m.sensor_id), m);
    }

    void send_to_queue(can::ids::SensorType type, can::ids::SensorId id,
                       const utils::TaskMessage &m) {
        switch (type) {
            case can::ids::SensorType::environment: {
                client.send_environment_sensor_queue(m);
                break;
            }
            case can::ids::SensorType::capacitive: {
                switch (id) {
                    case can::ids::SensorId::S1: {
                        client.send_capacitive_sensor_queue_rear(m);
                        break;
                    }
                    default:
                        client.send_capacitive_sensor_queue_front(m);
                        break;
                }
                break;
            }
            case can::ids::SensorType::pressure: {
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
