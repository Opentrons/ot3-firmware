#pragma once

#include "can/core/messages.hpp"

namespace gear_move_group_handler {

using namespace can::messages;

template <pipettes::tasks::move_group_task::TaskClient Client>
class GearMoveGroupHandler {
  public:
    using MessageType = std::variant<std::monostate, ClearAllMoveGroupsRequest,
                                     ExecuteMoveGroupRequest,
                                     GetMoveGroupRequest, TipActionRequest>;
    GearMoveGroupHandler(Client &task_client) : task_client{task_client} {}
    GearMoveGroupHandler(const GearMoveGroupHandler &) = delete;
    GearMoveGroupHandler(const GearMoveGroupHandler &&) = delete;
    auto operator=(const GearMoveGroupHandler &)
        -> GearMoveGroupHandler & = delete;
    auto operator=(const GearMoveGroupHandler &&)
        -> GearMoveGroupHandler && = delete;
    ~GearMoveGroupHandler() = default;

    void handle(MessageType &m) { task_client.send_move_group_queue(m); }

  private:
    Client &task_client;
};
}  // namespace gear_move_group_handler
