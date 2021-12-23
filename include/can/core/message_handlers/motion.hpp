#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace motion_message_handler {

using namespace can_messages;

template <motion_controller_task::TaskClient MotionTaskClient>
class MotionHandler {
  public:
    using MessageType =
        std::variant<std::monostate, DisableMotorRequest, EnableMotorRequest,
                     GetMotionConstraintsRequest, SetMotionConstraints,
                     StopRequest>;

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

}  // namespace motion_message_handler
