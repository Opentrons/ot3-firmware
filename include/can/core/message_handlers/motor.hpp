#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_message_handler {

using namespace can_message_writer;
using namespace can_ids;

template <class Motor>
class MotorHandler {
  public:
    using MessageType =
        std::variant<std::monostate, SetupRequest, StopRequest,
                     GetStatusRequest, MoveRequest, SetSpeedRequest,
                     GetSpeedRequest, SetAccelerationRequest,
                     GetAccelerationRequest>;

    MotorHandler(MessageWriter &message_writer, Motor &motor, NodeId node_id)
        : message_writer{message_writer}, motor{motor}, node_id{node_id} {}
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
        motor_messages::CanMove mv_msg{.target_position = m.target_position};
        motor.motion_controller.move(mv_msg);
    }

    void visit(EnableMotorRequest &m) {
        motor.motion_controller.enable_motor();
    }

    void visit(DisableMotorRequest &m) {
        motor.motion_controller.disable_motor();
    }

    void visit(SetSpeedRequest &m) {
        motor.motion_controller.set_velocity(m.speed);
    }

    void visit(GetSpeedRequest &m) {
        GetSpeedResponse response_msg{
            .speed = motor.motion_controller.get_current_velocity(),
            .node_id = node_id};
        message_writer.write(NodeId::host, response_msg);
    }

    void visit(SetAccelerationRequest &m) {
        motor.motion_controller.set_acceleration(m.acceleration);
    }

    void visit(GetAccelerationRequest &m) {
        GetAccelerationResponse response_msg{
            .acceleration = motor.motion_controller.get_current_acceleration(),
            .node_id = node_id};
        message_writer.write(NodeId::host, response_msg);
    }

    MessageWriter &message_writer;
    Motor &motor;
    NodeId node_id;
};

}  // namespace motor_message_handler