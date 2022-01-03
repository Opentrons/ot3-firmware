#include "gantry/core/can_task.hpp"

#include "can/core/can_bus.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/freertos_sender_task.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.hpp"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/utils.hpp"

using namespace can_dispatch;

static auto my_node_id = utils::get_node_id();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    freertos_sender_task::TaskMessage>{};

static auto message_writer_1 =
    can_message_writer::MessageWriter(can_sender_queue, my_node_id);

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
static auto dispatcher = can_task::GantryDispatcherType(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_group_executor_dispatch_target,
    device_info_dispatch_target);

auto static reader_message_buffer =
    freertos_can_dispatch::FreeRTOSCanBufferControl<
        can_task::reader_message_buffer_size, decltype(dispatcher)>{dispatcher};

auto static reader_task = can_task::CanMessageReaderTask{reader_message_buffer};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask,
                                can_bus::CanBus>{reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask,
                                can_bus::CanBus>{writer_task};

/**
 * Start the can reader task
 * @param canbus The can bus reference
 * @return The task entry point.
 */
auto can_task::start_reader(can_bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    LOG("Starting the CAN reader task\n");

    canbus.setup_node_id_filter(my_node_id);

    reader_task_control.start(5, "can_task", &canbus);

    return reader_task;
}

/**
 * Start the can writer task
 * @param canbus The can bus reference
 * @return The task entry point
 */
auto can_task::start_writer(can_bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    LOG("Starting the CAN writer task\n");

    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
