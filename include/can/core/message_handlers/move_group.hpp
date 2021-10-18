#pragma once

#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace move_group_handler {

using namespace can_message_writer;
using namespace can_messages;

template <typename MotionGroupManager>
class MoveGroupHandler {
  public:
    using MessageType =
        std::variant<std::monostate, AddLinearMoveRequest, GetMoveGroupRequest,
                     ClearMoveGroupRequest>;

    MoveGroupHandler(MessageWriter &message_writer,
                     MotionGroupManager &motion_group_manager)
        : message_writer{message_writer},
          motion_group_manager{motion_group_manager} {}
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
        message_writer.write(NodeId::host, m);
    }

    void visit(GetMoveGroupRequest &m) {
        message_writer.write(NodeId::host, m);
    }

    void visit(ClearMoveGroupRequest &m) {
        message_writer.write(NodeId::host, m);
    }

    MessageWriter &message_writer;
    MotionGroupManager &motion_group_manager;
};

}  // namespace move_group_handler