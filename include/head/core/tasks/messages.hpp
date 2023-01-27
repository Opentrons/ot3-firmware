#pragma once

#include "can/core/messages.hpp"

namespace presence_sensing_driver_task_messages {

/**
 * A message sent to check for new tool.
 */
struct CheckForToolChange {};

using TaskMessage =
    std::variant<std::monostate, can::messages::AttachedToolsRequest,
                 CheckForToolChange>;

}  // namespace presence_sensing_driver_task_messages
