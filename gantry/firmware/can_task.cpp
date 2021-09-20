#include "common/firmware/can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "can/core/message_writer.hpp"


//using namespace freertos_task;
using namespace hal_can_bus;
using namespace can_message_writer;

extern FDCAN_HandleTypeDef fdcan1;
static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);


