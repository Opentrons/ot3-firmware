#pragma once

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/freertos_sender_task.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/motor.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

using GantryDispatcherType = can_dispatch::Dispatcher<
    motor_message_handler::DispatchTarget<motor_class::Motor<lms::BeltConfig>>,
    move_group_handler::DispatchTarget,
    move_group_executor_handler::DispatchTarget<
        motor_class::Motor<lms::BeltConfig>>,
    device_info_handler::DispatchTarget>;

auto constexpr reader_message_buffer_size = 1024;
using CanReaderTaskEntry =
    freertos_can_dispatch::FreeRTOSCanReader<reader_message_buffer_size,
                                             GantryDispatcherType>;

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