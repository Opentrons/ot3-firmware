#pragma once

#include <variant>
#include "can/core/messages.hpp"
#include "motor-control/core/motion_group.hpp"

namespace move_group_task {

using TaskMessage = std::variant<std::monostate, can_messages::AddLinearMoveRequest,
      can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest, can_messages::ExecuteMoveGroupRequest>;

constexpr std::size_t max_groups = 6;
constexpr std::size_t max_moves_per_group = 5;

using MoveGroupType =
    move_group::MoveGroupManager<max_groups, max_moves_per_group,
                                 can_messages::AddLinearMoveRequest>;


/**
 * The handler of move group messages
 */
class MoveGroupMessageHandler {
  public:
    MoveGroupMessageHandler(MoveGroupType &move_group_manager)
        : move_groups{move_group_manager} {}
    ~MoveGroupMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m){this->handle(m);}, message);
    }

  private:
    void handle(std::monostate m) {
        static_cast<void>(m);
    }

    void handle(const can_messages::AddLinearMoveRequest& m) {
        static_cast<void>(move_groups[m.group_id].set_move(m));
    }

    void handle(const can_messages::GetMoveGroupRequest& m) {
        auto group = move_groups[m.group_id];
        auto response = can_messages::GetMoveGroupResponse{
            .group_id = m.group_id,
            .num_moves = static_cast<uint8_t>(group.size()),
            .total_duration = group.get_duration()};

//        message_writer.write(NodeId::host, response);
    }

    void handle(const can_messages::ClearAllMoveGroupsRequest& m) {
        for (auto &group : move_groups) {
            group.clear();
        }
    }

    void handle(const can_messages::ExecuteMoveGroupRequest& m) {
        auto group = move_groups[m.group_id];
        for (std::size_t i = 0; i < max_moves_per_group; i++) {
            auto move = group.get_move(i);
            std::visit([this](auto &m) { this->visit_move(m); }, move);
        }
    }

    void visit_move(const std::monostate &m) {

    }

    void visit_move(const can_messages::AddLinearMoveRequest &m) {
        //motor.motion_controller.move(m);
    }

    MoveGroupType& move_groups;
};

/**
 * The task type.
 */
using MoveGroupTask =
    freertos_message_queue_poller::FreeRTOSMessageQueuePoller<
        TaskMessage, MoveGroupMessageHandler>;

}