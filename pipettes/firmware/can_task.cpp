#include "pipettes/firmware/can_task.hpp"

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
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
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"
#include "pipettes/core/tasks.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

using namespace can_dispatch;

static auto& queue_client = pipettes_tasks::get_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto can_motor_handler = motor_message_handler::MotorHandler{queue_client};
static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler(queue_client);

static auto eeprom_handler = eeprom_message_handler::EEPromHandler{queue_client};
static auto device_info_message_handler =
    device_info_handler::DeviceInfoHandler{queue_client, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target =
    motor_message_handler::DispatchTarget{can_motor_handler};

static auto motion_group_dispatch_target =
    move_group_handler::DispatchTarget{can_move_group_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<decltype(eeprom_handler),
                        can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    device_info_handler::DispatchTarget{device_info_message_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    [](auto _) -> bool { return true; }, motor_dispatch_target,
    motion_group_dispatch_target, eeprom_dispatch_target,
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
void callback(void* cb_data, uint32_t identifier, uint8_t* data,
              uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
                                                 data + length);  // NOLINT
}


[[noreturn]] void can_task::CanMessageReaderTask::operator()(
    can_bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);
    can_start();
    can_bus->setup_node_id_filter(NodeId::pipette);
    // TODO (al, 2021-12-21): Move this out!!
    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    // TODO (al, 2021-12-21): Must account for this!!
//    motor.driver.setup();

    auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(read_can_message_buffer, dispatcher);
    poller();
}

auto static reader_task = can_task::CanMessageReaderTask{};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask, can_bus::CanBus>{
        reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask, can_bus::CanBus>{
        writer_task};

auto can_task::start_reader(can_bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

auto can_task::start_writer(can_bus::CanBus& canbus) -> CanMessageWriterTask& {
    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}