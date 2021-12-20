#include "common/firmware/tasks.hpp"

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/message_buffer.hpp"
#include "presence_sensor/core/presence_sensor_class.hpp"
#include "presence_sensor/core/presence_sensor_messages.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/adc.h"
#pragma GCC diagnostic pop
#include "common/firmware/adc_comms.hpp"
#include "common/firmware/errors.h"

using namespace freertos_task;
using namespace freertos_can_dispatch;
using namespace hal_can_bus;
using namespace can_message_writer;
using namespace can_ids;
using namespace can_dispatch;
using namespace presence_sensor_messages;
using namespace presence_sensing_message_handler;

using namespace can_parse;
using namespace can_message_buffer;
using namespace can_arbitration_id;
using namespace can_message_buffer;

adc::ADC_interface ADC_intf2 = {

    .ADC_handle = &adc2};

//adc::ADC ADC_comms2(ADC_intf2);

adc::ADC_interface ADC_intf1 = {

    .ADC_handle = &adc1

};

adc::ADC ADC_comms(ADC_intf1, ADC_intf2);
hal_can_bus::HalCanBus can_bus_1 =
    hal_can_bus::HalCanBus(can_get_device_handle());

// extern hal_can_bus::HalCanBus can_bus_1;
can_message_writer::MessageWriter message_writer_presence_sensor =
    MessageWriter(can_bus_1, NodeId::presence_sensor);

freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Reading>
    presence_sensor_queue("Presence Sensing Queue");
freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Ack_>
    complete_queue_presence_sensing("Presence Sensing Acked");
presence_sensor_class::PresenceSensor ps(ADC_comms, presence_sensor_queue,
                                         complete_queue_presence_sensing);
presence_sensing_message_handler::PresenceSensorHandler
    presence_sensing_handler =
        presence_sensing_message_handler::PresenceSensorHandler{
            message_writer_presence_sensor, ps};
#if 0
auto presence_sensor_dispatch_target =
    DispatchParseTarget<decltype(presence_sensing_handler),
                        can_messages::PresenceSensingRequest>{
        presence_sensing_handler};
#endif 

auto presence_sensor_dispatch_target =
    PresenceSenseDispatchTargetT{
        presence_sensing_handler};

//template <CanMessageBufferListener Listener>
auto build_ps_dispatch() -> can_dispatch::Dispatcher<PresenceSenseDispatchTargetT> {
    auto presence_sensor_dispatcher = Dispatcher(
        [](auto _) -> bool { return true; }, presence_sensor_dispatch_target);
    return presence_sensor_dispatcher;
}
