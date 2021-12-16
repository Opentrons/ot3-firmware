#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "presence_sensor/core/presence_sensor_messages.hpp"
#include "presence_sensor/core/presence_sensor_class.hpp"
#include "common/core/message_buffer.hpp"

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

extern can_message_writer::MessageWriter message_writer_presence_sensor;
extern freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Reading> presence_sensor_queue;
extern freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Ack>
    complete_queue_presence_sensing;
extern hal_can_bus::HalCanBus can_bus_1;
//extern presence_sensor_class::PresenceSensor ps;
//(ADC_comms2,presence_sensor_queue,complete_queue_presence_sensing);

template <typename... ArgTypes>
auto build(ArgTypes... args) -> std::tuple<ArgTypes...>;

template <typename NewElem, typename... TupleElem>
auto tuple_append(const std::tuple<TupleElem...> &tup, const NewElem &el) -> std::tuple<TupleElem..., NewElem>{
    return std::tuple_cat(tup, std::make_tuple(el));
}

template <CanMessageBufferListener Listener>
auto build_ps_dispatch() -> can_dispatch::Dispatcher<Listener>;