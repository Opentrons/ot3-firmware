#pragma once
#include <variant>

#include "motor-control/core/tasks/messages.hpp"
#include "spi/core/messages.hpp"

namespace tmc {
namespace tasks {

using SpiResponseMessage = std::tuple<spi::messages::TransactResponse>;
using CanMessageTuple = std::tuple<can::messages::ReadMotorDriverRegister,
                                   can::messages::WriteMotorDriverRegister,
                                   can::messages::WriteMotorCurrentRequest>;
using GearCanMessageTuple =
    std::tuple<can::messages::GearReadMotorDriverRegister,
               can::messages::GearWriteMotorDriverRegister,
               can::messages::GearWriteMotorCurrentRequest>;
using CanMessage =
    typename ::utils::TuplesToVariants<std::tuple<std::monostate>,
                                       CanMessageTuple>::type;
using GearCanMessage =
    typename ::utils::TuplesToVariants<std::tuple<std::monostate>,
                                       GearCanMessageTuple>::type;
using TaskMessage = typename ::utils::VariantCat<
    std::variant<std::monostate>,
    typename ::utils::TuplesToVariants<CanMessageTuple,
                                       SpiResponseMessage>::type>::type;

using GearTaskMessage = typename ::utils::VariantCat<
    std::variant<std::monostate>,
    typename ::utils::TuplesToVariants<GearCanMessageTuple,
                                       SpiResponseMessage>::type>::type;
/**
 * Concept describing a class that can message a motor driver task
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_motor_driver_queue(m)};
};

/**
 * Concept describing a class that can message a motor driver task
 * @tparam Client
 */
template <typename Client>
concept GearTaskClient = requires(Client client, const GearTaskMessage& m) {
    {client.send_motor_driver_queue(m)};
};
};  // namespace tasks
};  // namespace tmc
