#include "gantry/core/can_task.hpp"

#include <span>

#include "can/core/can_bus.hpp"
#include "can/core/can_writer_task.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "common/core/version.h"
#include "eeprom/core/message_handler.hpp"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"

using namespace can::dispatch;

static auto& queue_client = gantry::queues::get_queues();

static auto my_node_id = utils::get_node_id();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    can::message_writer_task::TaskMessage>{};

/** Handler for eeprom messages.*/
static auto eeprom_message_handler =
    eeprom::message_handler::EEPromHandler{queue_client, queue_client};
static auto eeprom_dispatch_target =
    can::dispatch::DispatchParseTarget<decltype(eeprom_message_handler),
                                       can::messages::WriteToEEPromRequest,
                                       can::messages::ReadFromEEPromRequest>{
        eeprom_message_handler};

/** The parsed message handler */
static auto can_motor_handler =
    can::message_handlers::motor::MotorHandler{queue_client};
static auto can_move_group_handler =
    can::message_handlers::move_group::MoveGroupHandler{queue_client};
static auto can_motion_handler =
    can::message_handlers::motion::MotionHandler{queue_client};

/** Handler of system messages. */
static auto system_message_handler =
    can::message_handlers::system::SystemMessageHandler{
        queue_client,
        version_get()->version,
        version_get()->flags,
        std::span(std::cbegin(version_get()->sha),
                  std::cend(version_get()->sha)),
        revision_get()->primary,
        revision_get()->secondary,
        reinterpret_cast<const std::array<char, 2>&>(revision_get()->tertiary)};
static auto system_dispatch_target =
    can_task::SystemDispatchTarget{system_message_handler};

static auto motor_dispatch_target =
    can_task::MotorDispatchTarget{can_motor_handler};

static auto motion_group_dispatch_target =
    can_task::MoveGroupDispatchTarget{can_move_group_handler};

static auto motion_dispatch_target =
    can_task::MotionControllerDispatchTarget{can_motion_handler};

can::dispatch::StandardArbIdTest core_arb_id{.node_id = my_node_id};
/** Dispatcher to the various handlers */
static auto dispatcher =
    can_task::GantryDispatcherType{core_arb_id,
                                   motor_dispatch_target,
                                   motion_group_dispatch_target,
                                   motion_dispatch_target,
                                   system_dispatch_target,
                                   eeprom_dispatch_target};

auto static reader_message_buffer =
    can::freertos_dispatch::FreeRTOSCanBufferControl<
        can_task::reader_message_buffer_size, decltype(dispatcher)>{dispatcher};

auto static reader_task = can_task::CanMessageReaderTask{reader_message_buffer};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask>{
        reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask>{
        writer_task};

/**
 * Start the can reader task
 * @param canbus The can bus reference
 * @return The task entry point.
 */
auto can_task::start_reader(can::bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    LOG("Starting the CAN reader task");

    canbus.setup_node_id_filter(my_node_id);

    reader_task_control.start(5, "can reader task", &canbus);

    return reader_task;
}

/**
 * Start the can writer task
 * @param canbus The can bus reference
 * @return The task entry point
 */
auto can_task::start_writer(can::bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    LOG("Starting the CAN writer task");

    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
