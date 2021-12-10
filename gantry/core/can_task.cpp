#include "can/core/can_bus.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_task.hpp"
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
static auto device_info_message_handler =
    device_info_handler::DeviceInfoHandler(message_writer_1, 0);
static auto device_info_dispatch_target =
    device_info_handler::DispatchTarget{device_info_message_handler};

static auto motor_dispatch_target =
    motor_message_handler::DispatchTarget{can_motor_handler};

static auto motion_group_dispatch_target =
    move_group_handler::DispatchTarget{can_move_group_handler};

static auto motion_group_executor_dispatch_target =
    move_group_executor_handler::DispatchTarget{
        can_move_group_executor_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_group_executor_dispatch_target,
    device_info_dispatch_target);

auto poller =
    freertos_can_dispatch::FreeRTOSCanReader<1024, decltype(dispatcher)>(
        can_bus_1, dispatcher);

[[noreturn]] void task_entry() {
    interfaces::initialize();

    can_bus_1.setup_node_id_filter(my_node_id);

    interfaces::get_motor().driver.setup();

    poller();
}

auto static task = freertos_task::FreeRTOSTask<512, 5>("can task", task_entry);
