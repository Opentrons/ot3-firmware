#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/move_group.hpp"
#include "motor-control/core/tasks/brushed_motion_controller_task.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace brushed_move_group_task {

constexpr std::size_t max_groups = 6;
constexpr std::size_t max_moves_per_group = 5;

using MoveGroupType =
    move_group::MoveGroupManager<max_groups, max_moves_per_group,
                                 can::messages::GripperGripRequest,
                                 can::messages::GripperHomeRequest,
                                 can::messages::AddBrushedLinearMoveRequest>;

using TaskMessage = motor_control_task_messages::BrushedMoveGroupTaskMessage;

/**
 * The handler of move group messages
 */
template <brushed_motion_controller_task::TaskClient MotionControllerClient,
          can::message_writer_task::TaskClient CanClient>
class MoveGroupMessageHandler {
  public:
    MoveGroupMessageHandler(MoveGroupType& move_group_manager,
                            MotionControllerClient& mc_client,
                            CanClient& can_client)
        : move_groups{move_group_manager},
          mc_client{mc_client},
          can_client{can_client} {}
    MoveGroupMessageHandler(const MoveGroupMessageHandler& c) = delete;
    MoveGroupMessageHandler(const MoveGroupMessageHandler&& c) = delete;
    auto operator=(const MoveGroupMessageHandler& c) = delete;
    auto operator=(const MoveGroupMessageHandler&& c) = delete;
    ~MoveGroupMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate&) {}

    void handle(const can::messages::GripperHomeRequest& m) {
        LOG("Received gripper home request: groupid=%d, seqid=%d\n", m.group_id,
            m.seq_id);
        static_cast<void>(move_groups[m.group_id].set_move(m));
    }

    void handle(const can::messages::GripperGripRequest& m) {
        LOG("Received gripper grip request: groupid=%d, seqid=%d\n", m.group_id,
            m.seq_id);
        static_cast<void>(move_groups[m.group_id].set_move(m));
    }

    void handle(const can::messages::AddBrushedLinearMoveRequest& m) {
        LOG("Received gripper move request: groupid=%d, seqid=%d\n", m.group_id,
            m.seq_id);
        static_cast<void>(move_groups[m.group_id].set_move(m));
    }

    void handle(const can::messages::GetMoveGroupRequest& m) {
        LOG("Received get move group request: groupid=%d", m.group_id);
        auto group = move_groups[m.group_id];
        auto response = can::messages::GetMoveGroupResponse{
            .group_id = m.group_id,
            .num_moves = static_cast<uint8_t>(group.size()),
            .total_duration = group.get_duration()};
        can_client.send_can_message(can::ids::NodeId::host, response);
    }

    void handle(const can::messages::ClearAllMoveGroupsRequest&) {
        LOG("Received clear move groups request");
        for (auto& group : move_groups) {
            group.clear();
        }
    }

    void handle(const can::messages::ExecuteMoveGroupRequest& m) {
        LOG("Received execute move group request: groupid=%d", m.group_id);
        auto group = move_groups[m.group_id];
        for (std::size_t i = 0; i < max_moves_per_group; i++) {
            auto move = group.get_move(i);
            std::visit([this](auto& m) { this->visit_move(m); }, move);
        }
    }

    void visit_move(const std::monostate& m) { static_cast<void>(m); }

    void visit_move(const can::messages::GripperGripRequest& m) {
        mc_client.send_brushed_motion_controller_queue(m);
    }

    void visit_move(const can::messages::GripperHomeRequest& m) {
        mc_client.send_brushed_motion_controller_queue(m);
    }
    void visit_move(const can::messages::AddBrushedLinearMoveRequest& m) {
        mc_client.send_brushed_motion_controller_queue(m);
    }

    MoveGroupType& move_groups;
    MotionControllerClient& mc_client;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MoveGroupTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    MoveGroupTask(QueueType& queue) : queue{queue} {}
    MoveGroupTask(const MoveGroupTask& c) = delete;
    MoveGroupTask(const MoveGroupTask&& c) = delete;
    auto operator=(const MoveGroupTask& c) = delete;
    auto operator=(const MoveGroupTask&& c) = delete;
    ~MoveGroupTask() = default;

    /**
     * Task entry point.
     */
    template <brushed_motion_controller_task::TaskClient MotionControllerClient,
              can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(MotionControllerClient* mc_client,
                                 CanClient* can_client) {
        auto handler =
            MoveGroupMessageHandler{move_group, *mc_client, *can_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
    MoveGroupType move_group{};
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_brushed_move_group_queue(m)};
};

}  // namespace brushed_move_group_task
