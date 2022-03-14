#pragma once

#include "can/core/messages.hpp"

namespace presence_sensing_driver_task_messages {

struct PollForChange {

};

using TaskMessage =
    std::variant<std::monostate,
                 can_messages::ReadPresenceSensingVoltageRequest,
                 can_messages::AttachedToolsRequest,
                 PollForChange>;

}  // namespace presence_sensing_driver_task_messages