#include "can/core/can_bus.hpp"
#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_task.hpp"
#include "gantry/core/gantry_motor.hpp"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/utils.hpp"

using namespace can_dispatch;

static auto my_axis_type = get_axis_type();
static auto my_node_id = utils::get_node_id();

static can_bus::CanBus& can_bus_1 = interfaces::get_can_bus();
static auto message_writer_1 =
    can_message_writer::MessageWriter(can_bus_1, my_node_id);

static auto move_group_manager = move_group_handler::MoveGroupType{};
/** The parsed message handler */
static auto can_motor_handler = motor_message_handler::MotorHandler{
    message_writer_1, interfaces::get_motor()};
static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler(message_writer_1, move_group_manager);
static auto can_move_group_executor_handler =
    move_group_executor_handler::MoveGroupExecutorHandler(
        message_writer_1, move_group_manager, interfaces::get_motor());

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

static auto motion_group_dispatch_target = DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest>{
    can_move_group_handler};

static auto motion_group_executor_dispatch_target =
    DispatchParseTarget<decltype(can_move_group_executor_handler),
                        can_messages::ExecuteMoveGroupRequest>{
        can_move_group_executor_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_group_executor_dispatch_target,
    device_info_dispatch_target);

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

[[noreturn]] void task_entry() {
    interfaces::initialize();

    can_bus_1.set_incoming_message_callback(callback);
    can_bus_1.setup_node_id_filter(my_node_id);

    interfaces::get_motor().driver.setup();

    auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, dispatcher);
    poller();
}

auto static task = freertos_task::FreeRTOSTask<512, 5>("can task", task_entry);
