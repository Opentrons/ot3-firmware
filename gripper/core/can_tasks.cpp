#include "can/core/can_bus.hpp"
#include "can/core/can_writer_task.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.hpp"
#include "gripper/core/can_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"

using namespace can_dispatch;

static auto& queue_client = gripper_tasks::get_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto can_motor_handler =
    motor_message_handler::MotorHandler{queue_client};
static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler{queue_client};
static auto can_motion_handler =
    motion_message_handler::MotionHandler{queue_client};

/** Handler of system messages. */
static auto system_message_handler =
    system_handler::SystemMessageHandler{queue_client, 0};
static auto system_dispatch_target =
    can_task::SystemDispatchTarget{system_message_handler};

static auto motor_dispatch_target =
    can_task::MotorDispatchTarget{can_motor_handler};

static auto motion_group_dispatch_target =
    can_task::MoveGroupDispatchTarget{can_move_group_handler};

static auto motion_dispatch_target =
    can_task::MotionControllerDispatchTarget{can_motion_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = can_task::GripperDispatcherType(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_dispatch_target,
    system_dispatch_target);

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

    canbus.setup_node_id_filter(can_ids::NodeId::gripper);

    reader_task_control.start(5, "can reader task", &canbus);

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
