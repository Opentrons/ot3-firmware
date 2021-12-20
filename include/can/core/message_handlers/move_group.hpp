#pragma once

#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

namespace move_group_handler {

using namespace can_messages;

template <move_group_task::TaskClient Client>
class MoveGroupHandler {
  public:
    using MessageType =
        std::variant<std::monostate, AddLinearMoveRequest,
                     ClearAllMoveGroupsRequest, ExecuteMoveGroupRequest,
                     GetMoveGroupRequest>;
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

/**
 * Type short cut for creating dispatch parse target for the handler.
 */
template <move_group_task::TaskClient Client>
using DispatchTarget = can_dispatch::DispatchParseTarget<
    MoveGroupHandler<Client>, AddLinearMoveRequest, ClearAllMoveGroupsRequest,
    ExecuteMoveGroupRequest, GetMoveGroupRequest>;

}  // namespace move_group_handler
