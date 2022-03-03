#pragma once

#include "can/core/messages.hpp"

namespace sensor_task_utils {

using TaskMessage =
    std::variant<std::monostate, can_messages::ReadFromSensorRequest,
                 can_messages::WriteToSensorRequest,
                 can_messages::BaselineSensorRequest,
                 can_messages::SetSensorThresholdRequest>;

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage &m) {
    {client.send_environment_sensor_queue(m)};
    {client.send_capacitive_sensor_queue(m)};
};

enum class BitMode : uint8_t { LSB = 0x0, MSB = 0x1 };

}  // namespace sensor_task_utils
