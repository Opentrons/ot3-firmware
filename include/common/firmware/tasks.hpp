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
#include "presence_sensor/core/presence_sensor_messages.hpp"
#include "presence_sensor/core/presence_sensor_class.hpp"


extern can_message_writer::MessageWriter message_writer_presence_sensor;
extern freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Reading> presence_sensor_queue;
extern freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Ack>
    complete_queue_presence_sensing;
extern hal_can_bus::HalCanBus can_bus_1;
//extern presence_sensor_class::PresenceSensor ps;
//(ADC_comms2,presence_sensor_queue,complete_queue_presence_sensing);

