#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "presence_sensor/core/presence_sensor_driver.hpp"
#include "presence_sensor/core/presence_sensor_messages.hpp"

namespace presence_sensing_message_handler {

using namespace can_message_writer;
using namespace can_messages;

template <class PresenceSensor>
class PresenceSensorHandler {
  public:
    PresenceSensorHandler(MessageWriter &message_writer,
                          PresenceSensor &presence_sensor)
        : message_writer{message_writer}, presence_sensor{presence_sensor} {}
    PresenceSensorHandler(const PresenceSensorHandler &) = delete;
    PresenceSensorHandler(const PresenceSensorHandler &&) = delete;
    auto operator=(const PresenceSensorHandler &)
        -> PresenceSensorHandler & = delete;
    auto operator=(const PresenceSensorHandler &&)
        -> PresenceSensorHandler && = delete;
    ~PresenceSensorHandler() = default;

    using MessageType = std::variant<std::monostate, PresenceSensingRequest>;
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(PresenceSensingRequest &m) {
        auto voltage_read = presence_sensor.driver.get_readings();

        GetPresenceSensingResponse response_msg{
            .z_motor = voltage_read.z_motor,
            .a_motor = voltage_read.a_motor,
            .gripper = voltage_read.gripper,
        };
        message_writer.write(NodeId::host, response_msg);
    }

    MessageWriter &message_writer;
    PresenceSensor &presence_sensor;
};

}  // namespace presence_sensing_message_handler
