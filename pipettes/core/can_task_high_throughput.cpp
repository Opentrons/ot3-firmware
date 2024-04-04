#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/version.h"
#include "pipettes/core/can_task.hpp"
#include "pipettes/core/dispatch_builder.hpp"

static auto& peripheral_queue_client = peripheral_tasks::get_queues();
static auto& sensor_queue_client = sensor_tasks::get_queues();
static auto& linear_motor_queue_client = linear_motor_tasks::get_queues();
static auto& linear_motor_driver_queue_client =
    linear_motor_tasks::tmc2160_driver::get_queues();
static auto& gear_left_queue_client = gear_motor_tasks::get_left_gear_queues();
static auto& gear_right_queue_client =
    gear_motor_tasks::get_right_gear_queues();
static auto& central_queue_client = central_tasks::get_queues();

/** The parsed message handler */
static auto can_motor_handler = can::message_handlers::motor::MotorHandler{
    linear_motor_driver_queue_client};
static auto can_motion_handler =
    can::message_handlers::motion::MotionHandler{linear_motor_queue_client};

static auto can_move_group_handler =
    can::message_handlers::move_group::MoveGroupHandler(
        linear_motor_queue_client);

static auto gear_motor_handler_left =
    can::message_handlers::motor::GearMotorHandler{gear_left_queue_client};
static auto gear_motor_handler_right =
    can::message_handlers::motor::GearMotorHandler{gear_right_queue_client};

static auto gear_motion_handler_left =
    gear_motion_handler::GearMotorMotionHandler{gear_left_queue_client};
static auto gear_motion_handler_right =
    gear_motion_handler::GearMotorMotionHandler{gear_right_queue_client};

static auto gear_move_group_handler_left =
    gear_move_group_handler::GearMoveGroupHandler(gear_left_queue_client);
static auto gear_move_group_handler_right =
    gear_move_group_handler::GearMoveGroupHandler(gear_right_queue_client);

static auto eeprom_handler = eeprom::message_handler::EEPromHandler{
    sensor_queue_client, sensor_queue_client};

static auto system_message_handler =
    can::message_handlers::system::SystemMessageHandler{
        central_queue_client, version_get()->version, version_get()->flags,
        std::span(std::cbegin(version_get()->sha),
                  std::cend(version_get()->sha)),
        revision_get()->primary, revision_get()->secondary,
        revision_get()->tertiary,
        // this is the high-throughput path so we can only be a 96
        static_cast<uint8_t>(can::ids::PipetteType::pipette_96)};

static auto sensor_handler =
    sensors::handlers::SensorHandler{sensor_queue_client};

static auto pipette_info_handler = pipette_info::PipetteInfoMessageHandler{
    central_queue_client, sensor_queue_client};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target =
    dispatch_builder::TMC2160MotorDispatchTarget{can_motor_handler};

static auto motion_controller_dispatch_target =
    dispatch_builder::MotionControllerDispatchTarget{can_motion_handler};

static auto motion_group_dispatch_target =
    dispatch_builder::MoveGroupDispatchTarget{can_move_group_handler};

static auto gear_motor_dispatch_target_left =
    dispatch_builder::GearMotorDispatchTarget{gear_motor_handler_left};
static auto gear_motor_dispatch_target_right =
    dispatch_builder::GearMotorDispatchTarget{gear_motor_handler_right};

static auto gear_motion_controller_dispatch_target_left =
    dispatch_builder::GearMotionControllerDispatchTarget{
        gear_motion_handler_left};
static auto gear_motion_controller_dispatch_target_right =
    dispatch_builder::GearMotionControllerDispatchTarget{
        gear_motion_handler_right};

static auto gear_motion_group_dispatch_target_left =
    dispatch_builder::GearMoveGroupDispatchTarget{gear_move_group_handler_left};
static auto gear_motion_group_dispatch_target_right =
    dispatch_builder::GearMoveGroupDispatchTarget{
        gear_move_group_handler_right};

static auto eeprom_dispatch_target =
    dispatch_builder::EEPromDispatchTarget{eeprom_handler};

static auto system_dispatch_target =
    dispatch_builder::SystemDispatchTarget{system_message_handler};

static auto sensor_dispatch_target =
    dispatch_builder::SensorDispatchTarget{sensor_handler};

static auto pipette_info_target =
    dispatch_builder::PipetteInfoDispatchTarget{pipette_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = can::dispatch::Dispatcher(
    [](auto) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_controller_dispatch_target,
    sensor_dispatch_target, eeprom_dispatch_target, pipette_info_target,
    system_dispatch_target, gear_motor_dispatch_target_left,
    gear_motion_controller_dispatch_target_left,
    gear_motion_group_dispatch_target_left, gear_motor_dispatch_target_right,
    gear_motion_controller_dispatch_target_right,
    gear_motion_group_dispatch_target_right);

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    can::message_writer_task::TaskMessage>{};

/**
 * The type of the message buffer populated by HAL ISR.
 */
static auto read_can_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto read_can_message_buffer_writer =
    can::message_buffer::CanMessageBufferWriter(read_can_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(void*, uint32_t identifier, uint8_t* data, uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
                                                 data + length);  // NOLINT
}

[[noreturn]] void can_task::CanMessageReaderTask::operator()(
    can::bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);
    can_bus->setup_node_id_filter(listen_id);

    auto poller = can::freertos_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, dispatcher);
    poller();
}

auto static reader_task =
    can_task::CanMessageReaderTask{can::ids::NodeId::pipette_left};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<256, can_task::CanMessageReaderTask>{
        reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<256, can_task::CanMessageWriterTask>{
        writer_task};

auto can_task::start_reader(can::bus::CanBus& canbus, can::ids::NodeId id)
    -> can_task::CanMessageReaderTask& {
    reader_task.listen_id = id;
    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

auto can_task::start_writer(can::bus::CanBus& canbus) -> CanMessageWriterTask& {
    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
