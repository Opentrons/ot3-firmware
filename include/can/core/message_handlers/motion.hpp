#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace can::message_handlers::motion {

using namespace can::messages;

template <motion_controller_task::TaskClient MotionTaskClient>
class MotionHandler {
  public:
    using MessageType =
        std::variant<std::monostate, DisableMotorRequest, EnableMotorRequest,
                     GetMotionConstraintsRequest, SetMotionConstraints,
                     ReadLimitSwitchRequest, MotorPositionRequest,
                     UpdateMotorPositionEstimationRequest, GetMotorUsageRequest,
                     MotorDriverErrorEncountered,
                     ResetMotorDriverErrorHandling>;

    MotionHandler(MotionTaskClient &motion_client)
        : motion_client{motion_client} {}
    MotionHandler(const MotionHandler &) = delete;
    MotionHandler(const MotionHandler &&) = delete;
    auto operator=(const MotionHandler &) -> MotionHandler & = delete;
    auto operator=(const MotionHandler &&) -> MotionHandler && = delete;
    ~MotionHandler() = default;

    void handle(MessageType &m) {
        std::visit([this](auto &message) { handle_message(message); }, m);
    }

  private:
    void handle_message(std::monostate &m) { static_cast<void>(m); }

    void handle_message(const auto &m) {
        motion_client.send_motion_controller_queue(m);
    }

    MotionTaskClient &motion_client;
};

template <brushed_motion_controller_task::TaskClient BrushedMotionTaskClient>
class BrushedMotionHandler {
  public:
    using MessageType =
        std::variant<std::monostate, DisableMotorRequest, EnableMotorRequest,
                     ReadLimitSwitchRequest, MotorPositionRequest,
                     SetGripperErrorToleranceRequest, GetMotorUsageRequest,
                     GripperJawStateRequest>;

    BrushedMotionHandler(BrushedMotionTaskClient &motion_client)
        : motion_client{motion_client} {}
    BrushedMotionHandler(const BrushedMotionHandler &) = delete;
    BrushedMotionHandler(const BrushedMotionHandler &&) = delete;
    auto operator=(const BrushedMotionHandler &)
        -> BrushedMotionHandler & = delete;
    auto operator=(const BrushedMotionHandler &&)
        -> BrushedMotionHandler && = delete;
    ~BrushedMotionHandler() = default;

    void handle(MessageType &m) {
        std::visit([this](auto &message) { handle_message(message); }, m);
    }

  private:
    void handle_message(std::monostate &m) { static_cast<void>(m); }

    void handle_message(const auto &m) {
        motion_client.send_brushed_motion_controller_queue(m);
    }

    BrushedMotionTaskClient &motion_client;
};

}  // namespace can::message_handlers::motion
