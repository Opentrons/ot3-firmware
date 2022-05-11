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
// TODO add message handler for gear queue client.
static auto& gear_queue_client = gear_motor_tasks::get_queues();
static auto& central_queue_client = central_tasks::get_queues();

/** The parsed message handler */
static auto can_motor_handler =
    motor_message_handler::MotorHandler{linear_motor_driver_queue_client};
static auto can_motion_handler =
    motion_message_handler::MotionHandler{linear_motor_queue_client};

static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler(linear_motor_queue_client);

static auto eeprom_handler = eeprom::message_handler::EEPromHandler{
    sensor_queue_client, sensor_queue_client};

static auto system_message_handler = system_handler::SystemMessageHandler{
    central_queue_client, version_get()->version, version_get()->flags,
    std::span(std::cbegin(version_get()->sha), std::cend(version_get()->sha))};

static auto sensor_handler =
    sensors::handlers::SensorHandler{sensor_queue_client};

static auto pipette_info_handler =
    pipette_info::PipetteInfoMessageHandler{central_queue_client};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target =
    dispatch_builder::TMC2160MotorDispatchTarget{can_motor_handler};

static auto motion_controller_dispatch_target =
    dispatch_builder::MotionControllerDispatchTarget{can_motion_handler};

static auto motion_group_dispatch_target =
    dispatch_builder::MoveGroupDispatchTarget{can_move_group_handler};

static auto eeprom_dispatch_target =
    dispatch_builder::EEPromDispatchTarget{eeprom_handler};

static auto system_dispatch_target =
    dispatch_builder::SystemDispatchTarget{system_message_handler};

static auto sensor_dispatch_target =
    dispatch_builder::SensorDispatchTarget{sensor_handler};

static auto pipette_info_target =
    dispatch_builder::PipetteInfoDispatchTarget{pipette_info_handler};

// TODO add dispatch target handler for gear queue client.
/** Dispatcher to the various handlers */
static auto dispatcher = can_dispatch::Dispatcher(
    [](auto) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, motion_controller_dispatch_target,
    sensor_dispatch_target, eeprom_dispatch_target, pipette_info_target,
    system_dispatch_target);

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

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
void callback(void*, uint32_t identifier, uint8_t* data, uint8_t length) {
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
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask>{
        reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask>{
        writer_task};

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
