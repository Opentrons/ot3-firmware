#pragma once

#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/motion_group.hpp"

namespace move_group_handler {

using namespace can_message_writer;
using namespace can_messages;

constexpr std::size_t max_groups = 6;
constexpr std::size_t max_moves_per_group = 5;

using MoveGroupType =
    move_group::MoveGroupManager<max_groups, max_moves_per_group,
                                 AddLinearMoveRequest>;

class MoveGroupHandler {
  public:
    using MessageType =
        std::variant<std::monostate, AddLinearMoveRequest, GetMoveGroupRequest,
                     ClearMoveGroupRequest, ClearAllMoveGroupsRequest>;
    MoveGroupHandler(MessageWriter &message_writer,
                     MoveGroupType &motion_group_manager)
        : message_writer{message_writer},
          motion_group_manager(motion_group_manager) {}
    MoveGroupHandler(const MoveGroupHandler &) = delete;
    MoveGroupHandler(const MoveGroupHandler &&) = delete;
    MoveGroupHandler &operator=(const MoveGroupHandler &) = delete;
    MoveGroupHandler &&operator=(const MoveGroupHandler &&) = delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(AddLinearMoveRequest &m) {
        motion_group_manager[m.group_id].set_move(m);
    }

    void visit(GetMoveGroupRequest &m) {
        auto group = motion_group_manager[m.group_id];
        // TODO (al, 2021-10-19): Get the real node id here.
        auto response = GetMoveGroupResponse{
            .group_id = m.group_id,
            .num_moves = static_cast<uint8_t>(group.size()),
            .total_duration = group.get_duration(),
            .node_id = static_cast<uint8_t>(can_ids::NodeId::host)};

        message_writer.write(NodeId::host, response);
    }

    void visit(ClearMoveGroupRequest &m) {
        auto& group = motion_group_manager[m.group_id];
        group.clear();
    }

    void visit(ClearAllMoveGroupsRequest &m) {
        for (auto& group : motion_group_manager) {
            group.clear();
        }
    }

    MessageWriter &message_writer;
    MoveGroupType &motion_group_manager;
};

}  // namespace move_group_handler
