#pragma once

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "hepa-uv/core/hepauv_info.hpp"
#include "hepa-uv/core/tasks.hpp"

namespace can_task {

using namespace hepauv_tasks;

using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        hepauv_tasks::QueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;
using HepaUVInfoDispatchTarget = can::dispatch::DispatchParseTarget<
    hepauv_info::HepaUVInfoMessageHandler<hepauv_tasks::QueueClient,
                                          hepauv_tasks::QueueClient>,
    can::messages::InstrumentInfoRequest, can::messages::SetSerialNumber>;

auto constexpr reader_message_buffer_size = 1024;

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
