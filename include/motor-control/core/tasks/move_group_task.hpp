#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/move_group.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace move_group_task {

constexpr std::size_t max_groups = 6;
constexpr std::size_t max_moves_per_group = 5;

using MoveGroupType =
    move_group::MoveGroupManager<max_groups, max_moves_per_group,
                                 can_messages::AddLinearMoveRequest>;

using TaskMessage = motor_control_task_messages::MoveGroupTaskMessage;

/**
 * The handler of move group messages
 */
template <typename AllTasks>
requires motion_controller_task::TaskClient<AllTasks>
class MoveGroupMessageHandler {
  public:
    MoveGroupMessageHandler(MoveGroupType& move_group_manager,
                            AllTasks& all_tasks)
        : move_groups{move_group_manager}, all_tasks{all_tasks} {}
    ~MoveGroupMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

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
        for (auto& group : move_groups) {
            group.clear();
        }
    }

    void handle(const can_messages::ExecuteMoveGroupRequest& m) {
        auto group = move_groups[m.group_id];
        for (std::size_t i = 0; i < max_moves_per_group; i++) {
            auto move = group.get_move(i);
            std::visit([this](auto& m) { this->visit_move(m); }, move);
        }
    }

    void visit_move(const std::monostate& m) {}

    void visit_move(const can_messages::AddLinearMoveRequest& m) {
        // motor.motion_controller.move(m);
    }

    MoveGroupType& move_groups;
    AllTasks& all_tasks;
};

/**
 * The task type.
 */
template <typename AllTasks>
requires motion_controller_task::TaskClient<AllTasks>
class MoveGroupTask {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;
    MoveGroupTask(QueueType& queue, MoveGroupType& move_group) : queue{queue}, move_group{move_group} {}
    ~MoveGroupTask() = default;

    /**
     * Task entry point.
     */
     [[noreturn]] void operator()(AllTasks* all_tasks) {
         auto handler = MoveGroupMessageHandler{move_group, *all_tasks};
         TaskMessage message{};
         for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
               handler.handle_message(message);
            }
         }
     }
   private:
     QueueType& queue;
     MoveGroupType& move_group;
};

/**
 * Concept describing a class that can message this task.
 * @tparam TaskHolder
 */
template <typename TaskHolder>
concept TaskClient = requires(TaskHolder holder, const TaskMessage& m) {
    {holder.send_move_group_queue(m)};
};

}  // namespace move_group_task