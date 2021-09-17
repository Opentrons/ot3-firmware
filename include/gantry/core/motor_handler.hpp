#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"

using namespace can_bus;
using namespace can_message_writer;
using namespace can_messages;

namespace motor_handler {

/**
 * Handler of CAN bus motor messages.
 */
template <CanBus CanBusType>
class MotorHandler {
    using MessageType =
        std::variant<std::monostate, SetSpeedRequest, GetSpeedRequest,
                     StopRequest, GetStatusRequest, MoveRequest>;

  public:
    MotorHandler(MessageWriter<CanBusType> &writer)
        : can_message_writer(writer) {}
    MotorHandler(const MotorHandler &) = delete;
    MotorHandler(const MotorHandler &&) = delete;
    MotorHandler &operator=(const MotorHandler &) = delete;
    MotorHandler &&operator=(const MotorHandler &&) = delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    /**
     * Handle the monostate.
     * @param m
     */
    void visit(std::monostate &m) {}

    /**
     * Handle set speed request.
     * @param m
     */
    void visit(SetSpeedRequest &m) {
        can_message_writer.write(NodeId::host, m);
    }

    /**
     * Handle get speed request.
     * @param m
     */
    void visit(GetSpeedRequest &m) {
        can_message_writer.write(NodeId::host, m);
    }

    /**
     * Handle stop request.
     * @param m
     */
    void visit(StopRequest &m) { can_message_writer.write(NodeId::gantry, m); }

    /**
     * Handle get status request.
     * @param m
     */
    void visit(GetStatusRequest &m) {
        can_message_writer.write(NodeId::host, m);
    }

    /**
     * Handle a move request.
     * @param m
     */
    void visit(MoveRequest &m) { can_message_writer.write(NodeId::host, m); }

    MessageWriter<CanBusType> can_message_writer;
};

}  // namespace motor_handler