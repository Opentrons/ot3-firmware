#pragma once
#include <variant>

#include "motor-control/core/tasks/messages.hpp"
#include "spi/core/messages.hpp"

namespace tmc {
namespace tasks {

using SpiResponseMessage = std::tuple<spi::messages::TransactResponse>;
using CanMessageTuple = std::tuple<can::messages::ReadMotorDriverRegister,
                                   can::messages::SetupRequest,
                                   can::messages::WriteMotorDriverRegister,
                                   can::messages::WriteMotorCurrentRequest>;
using CanMessage =
    typename ::utils::TuplesToVariants<std::tuple<std::monostate>,
                                       CanMessageTuple>::type;
using TaskMessage = typename ::utils::VariantCat<
    std::variant<std::monostate>,
    typename ::utils::TuplesToVariants<CanMessageTuple,
                                       SpiResponseMessage>::type>::type;

/**
 * Concept describing a class that can message a motor driver task
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_motor_driver_queue(m)};
};
};  // namespace tasks
};  // namespace tmc
