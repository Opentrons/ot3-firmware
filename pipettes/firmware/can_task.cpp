#include "common/firmware/can_task.hpp"

#include <variant>

#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_message_buffer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "common/firmware/spi_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"

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
using namespace move_group_handler;

extern FDCAN_HandleTypeDef fdcan1;

static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

static freertos_message_queue::FreeRTOSMessageQueue<Move> motor_queue(
    "Motor Queue");
static spi::Spi spi_comms{};

struct motion_controller::HardwareConfig PinConfigurations {
    .direction = {.port = GPIOB, .pin = GPIO_PIN_1},
    .step = {.port = GPIOA, .pin = GPIO_PIN_8},
    .enable = {.port = GPIOA, .pin = GPIO_PIN_10},
};

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 2},
        .steps_per_rev = 200,
        .microstep = 16},
    PinConfigurations, motor_queue};

static auto move_group_manager =
    move_group::MoveGroupManager<move_group_handler::max_moves_per_group,
                                 move_group_handler::max_groups>{};
/** The parsed message handler */
static auto can_motor_handler = MotorHandler{message_writer_1, motor};
static auto can_move_group_handler =
    MoveGroupHandler(message_writer_1, move_group_manager);

static auto i2c_comms = I2C{};
static auto eeprom_handler = EEPromHandler{message_writer_1, i2c_comms};
static auto device_info_handler =
    DeviceInfoHandler{message_writer_1, NodeId::pipette, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target =
    DispatchParseTarget<decltype(can_motor_handler), can_messages::SetupRequest,
                        can_messages::StopRequest,
                        can_messages::GetStatusRequest,
                        can_messages::MoveRequest>{can_motor_handler};

static auto motion_group_dispatch_target = DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::GetMoveGroupRequest, can_messages::ClearMoveGroupRequest>{
    can_move_group_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<decltype(eeprom_handler),
                        can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    DispatchParseTarget<DeviceInfoHandler, can_messages::DeviceInfoRequest>{
        device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher =
    Dispatcher(motor_dispatch_target, motion_group_dispatch_target,
               eeprom_dispatch_target, device_info_dispatch_target);

[[noreturn]] void task_entry() {
    if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
        Error_Handler();
    }
    can_bus::setup_node_id_filter(can_bus_1, NodeId::pipette);
    can_bus_1.start();

    auto poller = FreeRTOSCanBufferPoller(
        hal_can_message_buffer::get_message_buffer(), dispatcher);
    poller();
}

auto static task = FreeRTOSTask<512, 5>("can task", task_entry);

void can_task::start() {}
