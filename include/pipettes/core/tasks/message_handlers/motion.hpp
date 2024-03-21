#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "pipettes/core/tasks/messages.hpp"
#include "pipettes/core/tasks/motion_controller_task.hpp"

namespace gear_motion_handler {

using namespace can::messages;

template <
    pipettes::tasks::motion_controller_task::TaskClient GearMotionTaskClient>
class GearMotorMotionHandler {
  public:
    using MessageType =
        std::variant<std::monostate, GearDisableMotorRequest,
                     GearEnableMotorRequest, GetMotionConstraintsRequest,
                     SetMotionConstraints, ReadLimitSwitchRequest,
                     GetMotorUsageRequest, MotorStatusRequest>;

    GearMotorMotionHandler(GearMotionTaskClient &motion_client)
        : motion_client{motion_client} {}
    GearMotorMotionHandler(const GearMotorMotionHandler &) = delete;
    GearMotorMotionHandler(const GearMotorMotionHandler &&) = delete;
    auto operator=(const GearMotorMotionHandler &)
        -> GearMotorMotionHandler & = delete;
    auto operator=(const GearMotorMotionHandler &&)
        -> GearMotorMotionHandler && = delete;
    ~GearMotorMotionHandler() = default;

    void handle(MessageType &m) {
        std::visit([this](auto &message) { handle_message(message); }, m);
    }

  private:
    void handle_message(std::monostate &m) { static_cast<void>(m); }

    void handle_message(const auto &m) {
        motion_client.send_motion_controller_queue(m);
    }

    GearMotionTaskClient &motion_client;
};

}  // namespace gear_motion_handler
