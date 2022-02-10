#pragma once

#include "can/core/messages.hpp"

namespace sensor_task_utils {
using TaskMessage =
    std::variant<std::monostate, can_messages::ReadFromSensorRequest,
                 can_messages::WriteToSensorRequest,
                 can_messages::BaselineSensorRequest>;

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage &m) {
    {client.send_humidity_queue(m)};
};

}  // namespace sensor_task_utils
