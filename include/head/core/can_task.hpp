#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/can_writer_task.hpp"
#include "common/core/freertos_message_queue.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

struct CanMessageReaderTask {
    [[noreturn]] void operator()(can::bus::CanBus* can_bus);
};

/**
 * Create the can message reader task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_reader(can::bus::CanBus& canbus) -> CanMessageReaderTask&;

using CanMessageWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;

/**
 * Create the can message writer task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_writer(can::bus::CanBus& canbus) -> CanMessageWriterTask&;

}  // namespace can_task