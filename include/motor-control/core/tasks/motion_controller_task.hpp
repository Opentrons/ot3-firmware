#pragma once
#include <variant>
#include "can/core/messages.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "common/core/freertos_message_queue_poller.hpp"


namespace motion_controller_task {

using TaskMessage =
    std::variant<std::monostate, can_messages::StopRequest, can_messages::EnableMotorRequest,
                 can_messages::DisableMotorRequest, can_messages::GetMotionConstraintsRequest,
                 can_messages::SetMotionConstraints>;


template<lms::MotorMechanicalConfig MEConfig>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType = motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller): controller{controller} {}

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) {this->handle(m);}, message);
    }

  private:
    void handle(std::monostate m) {
        static_cast<void>(m);
    }

    void handle(const can_messages::EnableMotorRequest & m) {
        controller.stop();
    }

    void handle(const can_messages::DisableMotorRequest & m) {
        controller.disable_motor();
    }

    void handle(const can_messages::GetMotionConstraintsRequest & m) {
        auto constraints = controller.get_motion_constraints();
        can_messages::GetMotionConstraintsResponse response_msg{
            .min_velocity = constraints.min_velocity,
            .max_velocity = constraints.max_velocity,
            .min_acceleration = constraints.min_acceleration,
            .max_acceleration = constraints.max_acceleration,
        };
//        message_writer.write(NodeId::host, response_msg);
    }

    void handle(const can_messages::SetMotionConstraints & m) {
        controller.set_motion_constraints(m);
    }

    MotorControllerType & controller;
};


template<lms::MotorMechanicalConfig MEConfig>
using MotorControllerTask = freertos_message_queue_poller::FreeRTOSMessageQueuePoller<TaskMessage, MotionControllerMessageHandler<MEConfig>>;

}