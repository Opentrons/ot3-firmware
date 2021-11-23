#include <variant>

#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "common/firmware/spi_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

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
using namespace move_group_executor_handler;
using namespace motor_messages;
using namespace motor_driver_config;

static auto can_bus_1 = HalCanBus(can_get_device_handle());
static auto message_writer_1 = MessageWriter(can_bus_1, NodeId::pipette);

static freertos_message_queue::FreeRTOSMessageQueue<Move> motor_queue(
    "Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<Ack> complete_queue(
    "Complete Queue");

spi::SPI_interface SPI_intf = {

    .SPI_handle = &hspi2,
    .GPIO_handle = GPIOC,
    .pin = GPIO_PIN_8,
};
static spi::Spi spi_comms(SPI_intf);

struct motion_controller::HardwareConfig PinConfigurations {
    .direction = {.port = GPIOC,
                  .pin = GPIO_PIN_3,
                  .active_setting = GPIO_PIN_SET},
    .step = {.port = GPIOC, .pin = GPIO_PIN_7, .active_setting = GPIO_PIN_SET},
    .enable = {
        .port = GPIOC, .pin = GPIO_PIN_8, .active_setting = GPIO_PIN_SET},
};

RegisterConfig MotorDriverConfigurations{.gconf = 0x04,
                                         .ihold_irun = 0x70202,
                                         .chopconf = 0x101D5,
                                         .thigh = 0xFFFFF,
                                         .coolconf = 0x60000};
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
    PinConfigurations,
    MotionConstraints{.min_velocity = 1,
                      .max_velocity = 2,
                      .min_acceleration = 1,
                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue,
    complete_queue};

static auto move_group_manager = MoveGroupType{};
/** The parsed message handler */
static auto can_motor_handler = MotorHandler{message_writer_1, motor};
static auto can_move_group_handler =
    MoveGroupHandler(message_writer_1, move_group_manager);
static auto can_move_group_executor_handler =
    MoveGroupExecutorHandler(message_writer_1, move_group_manager, motor);

static auto i2c_comms = I2C{};
static auto eeprom_handler = EEPromHandler{message_writer_1, i2c_comms};
static auto device_info_handler = DeviceInfoHandler{message_writer_1, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target = DispatchParseTarget<
    decltype(can_motor_handler), can_messages::SetupRequest,
    can_messages::StopRequest, can_messages::EnableMotorRequest,
    can_messages::DisableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::WriteMotorDriverRegister,
    can_messages::ReadMotorDriverRegister>{can_motor_handler};

static auto motion_group_dispatch_target = DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest>{
    can_move_group_handler};

static auto motion_group_executor_dispatch_target =
    DispatchParseTarget<decltype(can_move_group_executor_handler),
                        can_messages::ExecuteMoveGroupRequest>{
        can_move_group_executor_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<decltype(eeprom_handler),
                        can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    DispatchParseTarget<DeviceInfoHandler, can_messages::DeviceInfoRequest>{
        device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    motor_dispatch_target, motion_group_dispatch_target, eeprom_dispatch_target,
    device_info_dispatch_target, motion_group_executor_dispatch_target);

/**
 * The type of the message buffer populated by HAL ISR.
 */
static auto read_can_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto read_can_message_buffer_writer =
    can_message_buffer::CanMessageBufferWriter(read_can_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(uint32_t identifier, uint8_t* data, uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
                                                 data + length);
}

[[noreturn]] void task_entry() {
    can_bus_1.set_incoming_message_callback(callback);
    can_start();
    can_bus_1.setup_node_id_filter(NodeId::pipette);

    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    motor.driver.setup();

    auto poller = FreeRTOSCanBufferPoller(read_can_message_buffer, dispatcher);
    poller();
}

auto static task = FreeRTOSTask<512, 5>("can task", task_entry);
