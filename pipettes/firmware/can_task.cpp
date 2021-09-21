#include "common/firmware/can_task.hpp"

#include <variant>

#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_message_buffer.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"
#include "pipettes/core/message_handlers/motor.hpp"

using namespace hal_can_bus;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace can_dispatch;
using namespace freertos_task;
using namespace can_message_writer;
using namespace i2c;
using namespace can_device_info;
using namespace eeprom_message_handler;
using namespace motor_message_handler;

extern FDCAN_HandleTypeDef fdcan1;

static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

template <typename T>
class MQ {
  public:
    bool try_send(const T, uint32_t timeout) { return true; }
    bool try_send(const T) { return true; }
    bool try_recv(T *) { return true; }
    bool has_message() const { return false; }
};

static auto mq = MQ<motor_command::Move>{};

/** The parsed message handler */
static auto motor_handler = MotorHandler{message_writer_1, mq};
static auto i2c_comms = I2C{};
static auto eeprom_handler = EEPromHandler{message_writer_1, i2c_comms};
static auto device_info_handler =
    DeviceInfoHandler{message_writer_1, NodeId::pipette, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target = DispatchParseTarget<
    decltype(motor_handler), can_messages::SetSpeedRequest,
    can_messages::GetSpeedRequest, can_messages::StopRequest,
    can_messages::GetStatusRequest, can_messages::MoveRequest>{motor_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<decltype(eeprom_handler),
                        can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    DispatchParseTarget<decltype(device_info_handler),
                        can_messages::DeviceInfoRequest>{device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    motor_dispatch_target, eeprom_dispatch_target, device_info_dispatch_target);

[[noreturn]] void task_entry() {
    if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
        Error_Handler();
    }
    can_bus_1.start();
    auto poller = FreeRTOSCanBufferPoller(
        hal_can_message_buffer::get_message_buffer(), dispatcher);
    poller();
}

auto static task = FreeRTOSTask<256, 5, void (*)()>("can task", task_entry);

void can_task::start() {}
