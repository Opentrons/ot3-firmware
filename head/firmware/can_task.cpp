#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/errors.h"
#include "common/firmware/spi_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

using namespace freertos_task;
using namespace freertos_can_dispatch;
using namespace hal_can_bus;
using namespace can_message_writer;
using namespace can_ids;
using namespace can_dispatch;
using namespace motor_message_handler;
using namespace move_group_handler;
using namespace move_group_executor_handler;
using namespace motor_messages;
using namespace motor_driver_config;
using namespace spi;

static auto can_bus_1 = HalCanBus(can_get_device_handle());
static auto message_writer_1 = MessageWriter(can_bus_1, NodeId::head);

static freertos_message_queue::FreeRTOSMessageQueue<Move> motor_queue(
    "Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<Ack> complete_queue(
    "Complete Queue");

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */

spi::SPI_interface SPI_intf2 = {

    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};
static spi::Spi spi_comms2(SPI_intf2);

spi::SPI_interface SPI_intf3 = {

    .SPI_handle = &hspi3,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOA,
    .pin = GPIO_PIN_4,
};
static spi::Spi spi_comms3(SPI_intf3);

struct motor_hardware::HardwareConfig pin_configurations_left {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_0,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_4,
        .active_setting = GPIO_PIN_SET},
};

struct motor_hardware::HardwareConfig pin_configurations_right {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_6,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB,
        .pin = GPIO_PIN_11,
        .active_setting = GPIO_PIN_RESET},
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

static motor_hardware::MotorHardware motor_hardware_right(
    pin_configurations_right, &htim7);
static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue, complete_queue, motor_hardware_right);

/*z motor would need a motor and PinConfigurations instance on its own*/
static motor_class::Motor motor_right{
    spi_comms3,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 20},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_hardware_right,
    MotionConstraints{.min_velocity = 1,
                      .max_velocity = 2,
                      .min_acceleration = 1,
                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue,
    complete_queue};

static motor_hardware::MotorHardware motor_hardware_left(
    pin_configurations_left, &htim7);
static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue, complete_queue, motor_hardware_left);

/*z motor*/
static motor_class::Motor motor_left{
    spi_comms2,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 20},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_hardware_left,
    MotionConstraints{.min_velocity = 1,
                      .max_velocity = 2,
                      .min_acceleration = 1,
                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue,
    complete_queue};

/** The parsed message handler */
static auto can_motor_handler = MotorHandler{message_writer_1, motor_right};
static auto can_motor_handler2 = MotorHandler{message_writer_1, motor_left};

static auto move_group_manager = MoveGroupType{};

static auto can_move_group_handler =
    MoveGroupHandler(message_writer_1, move_group_manager);

static auto can_move_group_executor_handler =
    MoveGroupExecutorHandler(message_writer_1, move_group_manager, motor_right);

static auto can_move_group_executor_handler2 =
    MoveGroupExecutorHandler(message_writer_1, move_group_manager, motor_left);

/** Handler of device info requests. */
static auto device_info_handler =
    can_device_info::DeviceInfoHandler(message_writer_1, 0);
static auto device_info_dispatch_target =
    DispatchParseTarget<decltype(device_info_handler),
                        can_messages::DeviceInfoRequest>{device_info_handler};

static auto motor_dispatch_target = DispatchParseTarget<
    decltype(can_motor_handler), can_messages::SetupRequest,
    can_messages::StopRequest, can_messages::EnableMotorRequest,
    can_messages::DisableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::WriteMotorDriverRegister,
    can_messages::ReadMotorDriverRegister>{can_motor_handler};

static auto motor_dispatch_target2 = DispatchParseTarget<
    decltype(can_motor_handler2), can_messages::SetupRequest,
    can_messages::StopRequest, can_messages::EnableMotorRequest,
    can_messages::DisableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::WriteMotorDriverRegister,
    can_messages::ReadMotorDriverRegister>{can_motor_handler2};

static auto motion_group_dispatch_target = DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest>{
    can_move_group_handler};

static auto motion_group_executor_dispatch_target =
    DispatchParseTarget<decltype(can_move_group_executor_handler),
                        can_messages::ExecuteMoveGroupRequest>{
        can_move_group_executor_handler};

static auto motion_group_executor_dispatch_target2 =
    DispatchParseTarget<decltype(can_move_group_executor_handler2),
                        can_messages::ExecuteMoveGroupRequest>{
        can_move_group_executor_handler2};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    motor_dispatch_target, motor_dispatch_target2, motion_group_dispatch_target,
    motion_group_executor_dispatch_target,
    motion_group_executor_dispatch_target2, device_info_dispatch_target);

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
                                                 data + length);  // NOLINT
}

extern "C" void motor_callback_glue() {
    motor_interrupt_left.run_interrupt();
    motor_interrupt_right.run_interrupt();
}

[[noreturn]] void task_entry() {
    can_bus_1.set_incoming_message_callback(callback);
    can_start();
    can_bus_1.setup_node_id_filter(NodeId::head);

    initialize_timer(motor_callback_glue);

    if (initialize_spi(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    if (initialize_spi(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    motor_left.driver.setup();
    motor_right.driver.setup();

    auto poller = FreeRTOSCanBufferPoller(read_can_message_buffer, dispatcher);
    poller();
}

auto static task = FreeRTOSTask<512, 5>("can task", task_entry);
