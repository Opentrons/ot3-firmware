#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/tasks/brushed_move_group_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

namespace can::message_handlers::move_group {

using namespace can::messages;

template <move_group_task::TaskClient Client>
class MoveGroupHandler {
  public:
    using MessageType =
        std::variant<std::monostate, AddLinearMoveRequest,
                     ClearAllMoveGroupsRequest, ExecuteMoveGroupRequest,
                     GetMoveGroupRequest, HomeRequest, StopRequest, AddSensorMoveRequest>;
    MoveGroupHandler(Client &task_client) : task_client{task_client} {}
    MoveGroupHandler(const MoveGroupHandler &) = delete;
    MoveGroupHandler(const MoveGroupHandler &&) = delete;
    auto operator=(const MoveGroupHandler &) -> MoveGroupHandler & = delete;
    auto operator=(const MoveGroupHandler &&) -> MoveGroupHandler && = delete;
    ~MoveGroupHandler() = default;

    void handle(MessageType &m) { task_client.send_move_group_queue(m); }

  private:
    Client &task_client;
};

template <brushed_move_group_task::TaskClient Brushed_Client>
class BrushedMoveGroupHandler {
  public:
    using MessageType =
        std::variant<std::monostate, ClearAllMoveGroupsRequest,
                     ExecuteMoveGroupRequest, GetMoveGroupRequest,
                     GripperGripRequest, GripperHomeRequest,
                     AddBrushedLinearMoveRequest, StopRequest>;
    BrushedMoveGroupHandler(Brushed_Client &task_client)
        : task_client{task_client} {}
    BrushedMoveGroupHandler(const BrushedMoveGroupHandler &) = delete;
    BrushedMoveGroupHandler(const BrushedMoveGroupHandler &&) = delete;
    auto operator=(const BrushedMoveGroupHandler &)
        -> BrushedMoveGroupHandler & = delete;
    auto operator=(const BrushedMoveGroupHandler &&)
        -> BrushedMoveGroupHandler && = delete;
    ~BrushedMoveGroupHandler() = default;

    void handle(MessageType &m) {
        task_client.send_brushed_move_group_queue(m);
    }

  private:
    Brushed_Client &task_client;
};

}  // namespace can::message_handlers::move_group
