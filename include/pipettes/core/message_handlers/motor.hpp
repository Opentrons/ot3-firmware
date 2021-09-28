#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"

using namespace can_bus;
using namespace can_message_writer;

namespace motor_message_handler {

template <can_bus::CanBusWriter Writer, class Motor>
class MotorHandler {
  public:
    using MessageType = std::variant<std::monostate, SetupRequest, StopRequest,
                                     GetStatusRequest, MoveRequest>;

    MotorHandler(MessageWriter<Writer> &message_writer, Motor &motor)
        : message_writer{message_writer}, motor{motor} {}
    MotorHandler(const MotorHandler &) = delete;
    MotorHandler(const MotorHandler &&) = delete;
    MotorHandler &operator=(const MotorHandler &) = delete;
    MotorHandler &&operator=(const MotorHandler &&) = delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(StopRequest &m) { motor.motion_controller.stop(); }

    void visit(SetupRequest &m) { motor.driver.setup(); }

    void visit(GetStatusRequest &m) {
        motor.driver.get_status();
        // TODO Format request
        GetStatusResponse response_msg{
            .status = motor.driver.get_current_status(),
            .data = motor.driver.get_current_data()};
        message_writer.write(NodeId::host, response_msg);
    }

    void visit(MoveRequest &m) {
        motor_messages::Move mv_msg{.steps = m.steps};
        motor.motion_controller.move(mv_msg);
    }

    MessageWriter<Writer> &message_writer;
    Motor &motor;
};

}  // namespace motor_message_handler