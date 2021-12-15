#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/freertos_sender_task.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"


namespace can_bus {
class CanBus;
}

namespace can_task {

struct CanReaderTaskEntry {
    CanReaderTaskEntry(can_bus::CanBus& bus);
    [[noreturn]] void operator()();
    can_bus::CanBus& can_bus;
};

auto constexpr reader_task_stack_depth = 512;
using CanMessageReaderTask =
    freertos_task::FreeRTOSTask<reader_task_stack_depth, CanReaderTaskEntry>;

/**
 * Create the can message reader task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_reader(can_bus::CanBus& canbus) -> CanMessageReaderTask;

auto constexpr writer_task_stack_depth = 512;
using CanMessageWriterTask = freertos_task::FreeRTOSTask<
    writer_task_stack_depth, freertos_sender_task::MessageSenderTask<
                                 freertos_message_queue::FreeRTOSMessageQueue>>;

/**
 * Create the can message writer task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_writer(can_bus::CanBus& canbus) -> CanMessageWriterTask;

}  // namespace can_task