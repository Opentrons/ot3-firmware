#pragma once

#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace motion_message_handler {

using namespace can_messages;

template <motion_controller_task::TaskClient MotionTaskClient>
class MotionHandler {
  public:
    using MessageType =
        std::variant<std::monostate, AddLinearMoveRequest, DisableMotorRequest,
                     EnableMotorRequest, GetMotionConstraintsRequest,
                     SetMotionConstraints, StopRequest>;

    MotionHandler(MotionTaskClient &motion_client)
        : motion_client{motion_client} {}
    MotionHandler(const MotionHandler &) = delete;
    MotionHandler(const MotionHandler &&) = delete;
    auto operator=(const MotionHandler &) -> MotionHandler & = delete;
    auto operator=(const MotionHandler &&) -> MotionHandler && = delete;
    ~MotionHandler() = default;

    void handle(MessageType &m) {
        motion_client.send_motion_controller_queue(m);
    }

  private:
    MotionTaskClient &motion_client;
};

/**
 * Type short cut for creating dispatch parse target for the handler.
 */
template <motion_controller_task::TaskClient MotionTaskClient>
using DispatchTarget = can_dispatch::DispatchParseTarget<
    MotionHandler<MotionTaskClient>, AddLinearMoveRequest, DisableMotorRequest,
    EnableMotorRequest, GetMotionConstraintsRequest, SetMotionConstraints,
    StopRequest>;

}  // namespace motion_message_handler
