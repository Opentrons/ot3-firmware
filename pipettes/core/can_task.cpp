#include "pipettes/core/can_task.hpp"

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"
#include "pipettes/core/tasks.hpp"

static auto& queue_client = pipettes_tasks::get_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto can_motor_handler =
    motor_message_handler::MotorHandler{queue_client};
static auto can_motion_handler =
    motion_message_handler::MotionHandler{queue_client};

static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler(queue_client);

static auto eeprom_handler =
    eeprom_message_handler::EEPromHandler{queue_client};
static auto system_message_handler =
    system_handler::SystemMessageHandler{queue_client, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target = can_dispatch::DispatchParseTarget<
    decltype(can_motor_handler), can_messages::ReadMotorDriverRegister,
    can_messages::SetupRequest, can_messages::WriteMotorDriverRegister>{
    can_motor_handler};

static auto motion_controller_dispatch_target =
    can_dispatch::DispatchParseTarget<
        decltype(can_motion_handler), can_messages::DisableMotorRequest,
        can_messages::EnableMotorRequest,
        can_messages::GetMotionConstraintsRequest,
        can_messages::SetMotionConstraints, can_messages::StopRequest,
        can_messages::ReadLimitSwitchRequest>{can_motion_handler};

static auto motion_group_dispatch_target = can_dispatch::DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest>{
    can_move_group_handler};

static auto eeprom_dispatch_target =
    can_dispatch::DispatchParseTarget<decltype(eeprom_handler),
                                      can_messages::WriteToEEPromRequest,
                                      can_messages::ReadFromEEPromRequest>{
        eeprom_handler};

static auto system_dispatch_target =
    can_dispatch::DispatchParseTarget<decltype(system_message_handler),
                                      can_messages::DeviceInfoRequest,
                                      can_messages::InitiateFirmwareUpdate>{
        system_message_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = can_dispatch::Dispatcher(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_controller_dispatch_target, motion_group_dispatch_target,
    eeprom_dispatch_target, system_dispatch_target);

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
void callback(void* cb_data, uint32_t identifier, uint8_t* data,
              uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
                                                 data + length);  // NOLINT
}

[[noreturn]] void can_task::CanMessageReaderTask::operator()(
    can_bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);
    can_bus->setup_node_id_filter(listen_id);

    auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, dispatcher);
    poller();
}

auto static reader_task =
    can_task::CanMessageReaderTask{can_ids::NodeId::pipette_left};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask,
                                can_bus::CanBus>{reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask,
                                can_bus::CanBus>{writer_task};

auto can_task::start_reader(can_bus::CanBus& canbus, can_ids::NodeId id)
    -> can_task::CanMessageReaderTask& {
    reader_task.listen_id = id;
    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

auto can_task::start_writer(can_bus::CanBus& canbus) -> CanMessageWriterTask& {
    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
