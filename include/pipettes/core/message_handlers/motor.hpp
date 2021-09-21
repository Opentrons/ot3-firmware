#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/move_message.hpp"

using namespace can_bus;
using namespace can_message_writer;

namespace motor_message_handler {

template <can_bus::CanBusWriter Writer, MessageQueue<motor_command::Move> Queue>
class MotorHandler {
  public:
    using MessageType =
        std::variant<std::monostate, SetSpeedRequest, GetSpeedRequest,
                     StopRequest, GetStatusRequest, MoveRequest>;

    MotorHandler(MessageWriter<Writer> &message_writer, Queue &message_queue)
        : message_writer{message_writer}, message_queue{message_queue} {}
    MotorHandler(const MotorHandler &) = delete;
    MotorHandler(const MotorHandler &&) = delete;
    MotorHandler &operator=(const MotorHandler &) = delete;
    MotorHandler &&operator=(const MotorHandler &&) = delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(SetSpeedRequest &m) { message_writer.write(NodeId::host, m); }

    void visit(GetSpeedRequest &m) { message_writer.write(NodeId::host, m); }

    void visit(StopRequest &m) { message_writer.write(NodeId::gantry, m); }

    void visit(GetStatusRequest &m) { message_writer.write(NodeId::host, m); }

    void visit(MoveRequest &m) { message_writer.write(NodeId::host, m); }

    MessageWriter<Writer> &message_writer;
    Queue &message_queue;
};

}  // namespace motor_message_handler