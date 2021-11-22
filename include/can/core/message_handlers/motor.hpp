#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_message_handler {

using namespace can_message_writer;
using namespace can_messages;
using namespace motor_driver_config;

template <class Motor>
class MotorHandler {
  public:
    using MessageType =
        std::variant<std::monostate, SetupRequest, StopRequest,
                     GetStatusRequest, MoveRequest, EnableMotorRequest,
                     DisableMotorRequest, GetMotionConstraintsRequest,
                     SetMotionConstraints, WriteMotorDriverRegister,
                     ReadMotorDriverRegister>;

    MotorHandler(MessageWriter &message_writer, Motor &motor)
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

    void visit(MoveRequest &m) { motor.motion_controller.move(m); }

    void visit(EnableMotorRequest &m) {
        motor.motion_controller.enable_motor();
    }

    void visit(DisableMotorRequest &m) {
        motor.motion_controller.disable_motor();
    }

    void visit(GetMotionConstraintsRequest &m) {
        auto constraints = motor.motion_controller.get_motion_constraints();
        GetMotionConstraintsResponse response_msg{
            .min_velocity = constraints.min_velocity,
            .max_velocity = constraints.max_velocity,
            .min_acceleration = constraints.min_acceleration,
            .max_acceleration = constraints.max_acceleration,
        };
        message_writer.write(NodeId::host, response_msg);
    }

    void visit(SetMotionConstraints &m) {
        motor.motion_controller.set_motion_constraints(m);
    }

    void visit(WriteMotorDriverRegister &m) {
        if (DriverRegisters::is_valid_address(m.reg_address)) {
            motor.driver.write(DriverRegisters::Addresses(m.reg_address),
                               m.data);
        }
    }

    void visit(ReadMotorDriverRegister &m) {
        uint32_t data;
        if (DriverRegisters::is_valid_address(m.reg_address)) {
            motor.driver.read(DriverRegisters::Addresses(m.reg_address), data);
        }
        ReadMotorDriverRegisterResponse response_msg{
            .reg_address = m.reg_address,
            .data = data,
        };
        message_writer.write(NodeId::host, response_msg);
    }

    MessageWriter &message_writer;
    Motor &motor;
};

}  // namespace motor_message_handler