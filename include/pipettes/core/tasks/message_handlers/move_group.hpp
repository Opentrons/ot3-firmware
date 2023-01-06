#pragma once

#include "can/core/messages.hpp"
#include "pipettes/core/tasks/messages.hpp"

namespace gear_move_group_handler {

using namespace can::messages;

template <pipettes::tasks::move_group_task::TaskClient Client>
class GearMoveGroupHandler {
  public:
    using MessageType =
        pipettes::task_messages::move_group_task_messages::MoveGroupTaskMessage;
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
