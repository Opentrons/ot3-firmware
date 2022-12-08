#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/tasks/brushed_motor_driver_task.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"

namespace can::message_handlers::motor {

using namespace can::messages;

template <tmc::tasks::TaskClient Client>
class MotorHandler {
  public:
    using MessageType = tmc::tasks::CanMessage;

    MotorHandler(Client &motor_client) : motor_client{motor_client} {}
    MotorHandler(const MotorHandler &) = delete;
    MotorHandler(const MotorHandler &&) = delete;
    auto operator=(const MotorHandler &) -> MotorHandler & = delete;
    auto operator=(const MotorHandler &&) -> MotorHandler && = delete;
    ~MotorHandler() = default;

    void handle(MessageType &can_message) {
        std::visit(
            [this](auto m) -> void {
                this->motor_client.send_motor_driver_queue(m);
            },
            can_message);
    }

  private:
    Client &motor_client;
};

template <brushed_motor_driver_task::TaskClient BrushedMotorDriverTaskClient>
class BrushedMotorHandler {
  public:
    using MessageType =
        std::variant<std::monostate, SetBrushedMotorVrefRequest,
                     SetBrushedMotorPwmRequest, BrushedMotorConfRequest>;

    BrushedMotorHandler(BrushedMotorDriverTaskClient &motor_client)
        : motor_client{motor_client} {}
    BrushedMotorHandler(const BrushedMotorHandler &) = delete;
    BrushedMotorHandler(const BrushedMotorHandler &&) = delete;
    auto operator=(const BrushedMotorHandler &)
        -> BrushedMotorHandler & = delete;
    auto operator=(const BrushedMotorHandler &&)
        -> BrushedMotorHandler && = delete;
    ~BrushedMotorHandler() = default;

    void handle(MessageType &m) {
        motor_client.send_brushed_motor_driver_queue(m);
    }

  private:
    BrushedMotorDriverTaskClient &motor_client;
};

}  // namespace can::message_handlers::motor
