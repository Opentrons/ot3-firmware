#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/motion_group.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "move_group.hpp"

namespace move_group_executor_handler {

using namespace can_message_writer;
using namespace can_messages;
using namespace motor_messages;

template <typename Motor>
struct TaskEntry {
    MessageWriter &message_writer;
    Motor &motor;
    NodeId node_id;
    void operator()() {
        while (true) {
            auto try_read = Ack{};
            if (motor.completed_move_queue.try_read(&try_read, portMAX_DELAY)) {
                MoveCompleted msg = {
                    .group_id = try_read.group_id,
                    .seq_id = try_read.seq_id,
                    .current_position = try_read.current_position,
                    .ack_id = static_cast<uint8_t>(
                        motor_messages::AckMessageId::complete),
                };
                message_writer.write(NodeId::host, msg);
            }
        }
    }
};
template <typename Motor>
class MoveGroupExecutorHandler {
  public:
    using MessageType = std::variant<std::monostate, ExecuteMoveGroupRequest>;

    MoveGroupExecutorHandler(
        MessageWriter &message_writer,
        move_group_handler::MoveGroupType &motion_group_manager, Motor &motor,
        NodeId node_id)
        : message_writer{message_writer},
          motion_group_manager{motion_group_manager},
          motor(motor),
          node_id(node_id),
          task_entry{message_writer, motor, node_id},
          ack_task("ack task", task_entry) {}
    MoveGroupExecutorHandler(const MoveGroupExecutorHandler &) = delete;
    MoveGroupExecutorHandler(const MoveGroupExecutorHandler &&) = delete;
    MoveGroupExecutorHandler &operator=(const MoveGroupExecutorHandler &) =
        delete;
    MoveGroupExecutorHandler &&operator=(const MoveGroupExecutorHandler &&) =
        delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(ExecuteMoveGroupRequest &m) {
        auto group = motion_group_manager[m.group_id];
        for (std::size_t i = 0; i < move_group_handler::max_moves_per_group;
             i++) {
            auto move = group.get_move(i);
            std::visit([this](auto &m) { this->visit_move(m); }, move);
        }
    }

    void visit_move(const std::monostate &m) {}

    void visit_move(const AddLinearMoveRequest &m) {
        motor.motion_controller.move(m);
    }

    MessageWriter &message_writer;
    move_group_handler::MoveGroupType &motion_group_manager;
    Motor &motor;
    NodeId node_id;
    TaskEntry<Motor> task_entry;
    freertos_task::FreeRTOSTask<512, 5> ack_task;
};

}  // namespace move_group_executor_handler