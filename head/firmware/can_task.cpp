#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "can/firmware/hal_can_message_buffer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "common/firmware/spi_comms.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "pipettes/core/message_handlers/motor.hpp"

using namespace freertos_task;
using namespace freertos_can_dispatch;
using namespace hal_can_bus;
using namespace can_message_writer;
using namespace can_ids;
using namespace can_dispatch;
using namespace motor_message_handler;

extern FDCAN_HandleTypeDef fdcan1;
static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

/**
 * Set up the can bus receive filters.
 *
 * @param can_filters CanBusFilters interface.
 */
static void can_filter_setup(CanBusFilters& can_filters) {
    // The node id mask.
    auto node_id_mask = ArbitrationId{
        .parts = {.function_code = 0, .node_id = 0xFFFF, .message_id = 0}};

    // Set up the broadcast filter
    auto filter = ArbitrationId{.id = 0};
    filter.parts.node_id = static_cast<uint32_t>(NodeId::broadcast);
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0,
                           filter.id, node_id_mask.id);

    // Set up the head filter
    filter.id = 0;
    filter.parts.node_id = static_cast<uint32_t>(NodeId::head);
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1,
                           filter.id, node_id_mask.id);

    // Reject everything else
    can_filters.add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);
}

static freertos_message_queue::FreeRTOSMessageQueue<Move> motor_queue(
    "Motor Queue");
static spi::Spi spi_comms{};

struct motion_controller::HardwareConfig PinConfigurations {
    .direction = {.port = GPIOB, .pin = GPIO_PIN_1},
    .step = {.port = GPIOA, .pin = GPIO_PIN_8},
    .enable = {.port = GPIOA, .pin = GPIO_PIN_10},
};

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::BeltConfig>{
        .mech_config =
            lms::BeltConfig{.belt_pitch = 2, .pulley_tooth_count = 10},
        .steps_per_rev = 200,
        .microstep = 16},
    PinConfigurations, motor_queue};

/** The parsed message handler */
static auto can_motor_handler = MotorHandler{message_writer_1, motor};

/** Handler of device info requests. */
static auto device_info_handler =
    can_device_info::DeviceInfoHandler(message_writer_1, NodeId::head, 0);
static auto device_info_dispatch_target =
    DispatchParseTarget<decltype(device_info_handler),
                        can_messages::DeviceInfoRequest>{device_info_handler};

static auto motor_dispatch_target =
    DispatchParseTarget<decltype(can_motor_handler), can_messages::SetupRequest,
                        can_messages::StopRequest,
                        can_messages::GetStatusRequest,
                        can_messages::MoveRequest>{can_motor_handler};
/** Dispatcher to the various handlers */
static auto dispatcher =
    Dispatcher(motor_dispatch_target, device_info_dispatch_target);

[[noreturn]] void task_entry() {
    if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
        Error_Handler();
    }
    can_filter_setup(can_bus_1);
    can_bus_1.start();

    auto poller = FreeRTOSCanBufferPoller(
        hal_can_message_buffer::get_message_buffer(), dispatcher);
    poller();
}

auto static task = FreeRTOSTask<256, 5>("can task", task_entry);
