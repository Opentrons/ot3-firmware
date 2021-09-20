#include "common/firmware/can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/device_info.hpp"
#include "can/core/ids.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"


//using namespace freertos_task;
using namespace hal_can_bus;
using namespace can_message_writer;
using namespace can_ids;
using namespace can_dispatch;

extern FDCAN_HandleTypeDef fdcan1;
static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

/** Handler of device info requests. */
static auto device_info_handler = can_device_info::DeviceInfoHandler(message_writer_1, NodeId::gantry, 0);
static auto device_info_dispatch_target =
    DispatchParseTarget<decltype(device_info_handler),
    can_messages::DeviceInfoRequest>{device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(device_info_dispatch_target);


