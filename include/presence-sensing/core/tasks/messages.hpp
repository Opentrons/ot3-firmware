#pragma once

#include "can/core/messages.hpp"
#include "presence-sensing/core/presence_sensing_messages.hpp"

namespace presence_sensing_task_messages {

using PresenceSensingTaskMessage = presence_sensing_messages::GetVoltage;

using PresenceSensingAckTaskMessage = presence_sensing_messages::Ack;

}  // presence_sensing_task_messages 