#include "common/firmware/can_task.hpp"

#include <variant>

#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/move_group_executor.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_message_buffer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "common/firmware/spi_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "pipettes/core/eeprom.hpp"
#include "pipettes/core/message_handlers/eeprom.hpp"
#include "platform_specific_hal_conf.h"

using namespace hal_can_bus;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace can_dispatch;
using namespace freertos_task;
using namespace can_message_writer;
using namespace i2c;
using namespace can_device_info;
using namespace eeprom_message_handler;
using namespace motor_message_handler;
using namespace move_group_handler;
using namespace move_group_executor_handler;
using namespace motor_messages;

extern FDCAN_HandleTypeDef fdcan1;

static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

static freertos_message_queue::FreeRTOSMessageQueue<Move> motor_queue(
    "Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<Ack> complete_queue(
    "Complete Queue");

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PC6     ------> SPI2_CS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_CIPO
        PB15     ------> SPI2_COPI

         Step/Dir
         PC3  ---> Dir Pin
         PC7  ---> Step Pin
         Enable
         PC8  ---> Enable Pin
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // Enable/Dir/Step pin
        GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
}

SPI_HandleTypeDef MX_SPI2_Init() {
    /* SPI2 parameter configuration*/
    __HAL_RCC_SPI2_CLK_ENABLE();
    SPI_HandleTypeDef hspi2 = {
        .Instance = SPI2,
        .Init = {.Mode = SPI_MODE_MASTER,
                 .Direction = SPI_DIRECTION_2LINES,
                 .DataSize = SPI_DATASIZE_8BIT,
                 .CLKPolarity = SPI_POLARITY_HIGH,
                 .CLKPhase = SPI_PHASE_2EDGE,
                 .NSS = SPI_NSS_SOFT,
                 .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                 .FirstBit = SPI_FIRSTBIT_MSB,
                 .TIMode = SPI_TIMODE_DISABLE,
                 .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                 .CRCPolynomial = 7,
                 .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                 .NSSPMode = SPI_NSS_PULSE_DISABLE}

    };

    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    return hspi2;
}
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
        */
        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8 | GPIO_PIN_9);
    }
}

SPI_HandleTypeDef handle = MX_SPI2_Init();

spi::SPI_interface SPI_intf = {

    .SPI_handle = &handle,
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};
static spi::Spi spi_comms(SPI_intf);

struct motion_controller::HardwareConfig PinConfigurations {
    .direction = {.port = GPIOC, .pin = GPIO_PIN_3},
    .step = {.port = GPIOC, .pin = GPIO_PIN_7},
    .enable = {.port = GPIOC, .pin = GPIO_PIN_8},
};

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 2},
        .steps_per_rev = 200,
        .microstep = 16},
    PinConfigurations, motor_queue, complete_queue};

static auto move_group_manager = MoveGroupType{};
/** The parsed message handler */
static auto can_motor_handler = MotorHandler{message_writer_1, motor};
static auto can_move_group_handler =
    MoveGroupHandler(message_writer_1, move_group_manager);
static auto can_move_group_executor_handler = MoveGroupExecutorHandler(
    message_writer_1, move_group_manager, motor, NodeId::pipette);

static auto i2c_comms = I2C{};
static auto eeprom_handler = EEPromHandler{message_writer_1, i2c_comms};
static auto device_info_handler =
    DeviceInfoHandler{message_writer_1, NodeId::pipette, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target = DispatchParseTarget<
    decltype(can_motor_handler), can_messages::SetupRequest,
    can_messages::StopRequest, can_messages::GetStatusRequest,
    can_messages::MoveRequest, can_messages::EnableMotorRequest,
    can_messages::DisableMotorRequest>{can_motor_handler};

static auto motion_group_dispatch_target = DispatchParseTarget<
    decltype(can_move_group_handler), can_messages::AddLinearMoveRequest,
    can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest>{
    can_move_group_handler};

static auto motion_group_executor_dispatch_target =
    DispatchParseTarget<decltype(can_move_group_executor_handler),
                        can_messages::ExecuteMoveGroupRequest>{
        can_move_group_executor_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<decltype(eeprom_handler),
                        can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    DispatchParseTarget<DeviceInfoHandler, can_messages::DeviceInfoRequest>{
        device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    motor_dispatch_target, motion_group_dispatch_target, eeprom_dispatch_target,
    device_info_dispatch_target, motion_group_executor_dispatch_target);

[[noreturn]] void task_entry() {
    if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
        Error_Handler();
    }
    can_bus::setup_node_id_filter(can_bus_1, NodeId::pipette);
    can_bus_1.start();

    auto poller = FreeRTOSCanBufferPoller(
        hal_can_message_buffer::get_message_buffer(), dispatcher);
    poller();
}

auto static task = FreeRTOSTask<512, 5>("can task", task_entry);

void can_task::start() {}
