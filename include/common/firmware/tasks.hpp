#pragma once

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "presence_sensor/core/presence_sensor_class.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/message_buffer.hpp"
#include "common/firmware/adc_comms.hpp"


using namespace freertos_task;
using namespace freertos_can_dispatch;
using namespace hal_can_bus;
using namespace can_message_writer;
using namespace can_ids;
using namespace can_dispatch;


using namespace can_parse;
using namespace can_message_buffer;
using namespace can_arbitration_id;
using namespace can_message_buffer;

using PresenceSenseDispatchTargetT = DispatchParseTarget<
                presence_sensing_message_handler::PresenceSensorHandler,
                can_messages::PresenceSensingRequest>;
#if 0
template <CanMessageBufferListener Listener>
auto build_ps_dispatch() -> decltype(can_dispatch::Dispatcher<Listener>);
#endif 

auto build_ps_dispatch() -> can_dispatch::Dispatcher<PresenceSenseDispatchTargetT>;
