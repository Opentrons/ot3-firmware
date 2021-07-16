#include "common/firmware/can_task.hpp"

#include <array>

#include "FreeRTOS.h"
#include "can/core/messages.hpp"
#include "can/core/parse.hpp"
#include "common/firmware/can.h"
#include "task.h"

static void Error_Handler();

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static FDCAN_TxHeaderTypeDef TxHeader;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static FDCAN_RxHeaderTypeDef RxHeader;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static FDCAN_HandleTypeDef can;
static constexpr auto buffsize = 64;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto buff = std::array<uint8_t, buffsize>{};


template <typename... MessageTypes>
class MessageHandler {
  public:
    explicit MessageHandler()  {}

    void handle(FDCAN_HandleTypeDef *can_handle, can_ids::MessageId message_id,
                can_messages::BodyType &body) {
        auto message = parser.parse(message_id, body);
        auto handle = [this, can_handle](auto m) { this->handle(m, can_handle); };
        std::visit(handle, message);
    }

  private:
    void handle(const can_messages::MoveRequest &m, FDCAN_HandleTypeDef *can_handle) {
        static_cast<void>(m);
    }

    void handle(const can_messages::SetSpeedRequest &m, FDCAN_HandleTypeDef *can_handle) {
        speed = m.mm_sec;
    }

    void handle(const can_messages::SetupRequest &m, FDCAN_HandleTypeDef *can_handle){

    }

    void handle(const can_messages::GetStatusRequest &m, FDCAN_HandleTypeDef *can_handle) {
        TxHeader.Identifier = static_cast<uint32_t >(m.id);
        TxHeader.IdType = FDCAN_EXTENDED_ID;
        TxHeader.TxFrameType = FDCAN_DATA_FRAME;
        TxHeader.DataLength = FDCAN_DLC_BYTES_5;
        TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
        TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
        TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
        TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        TxHeader.MessageMarker = 0;

        auto response = can_messages::GetStatusResponse{1, 2};

        auto sp = BodyType(buff);
        response.serialize(sp);

        HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data());
    }

    void handle(const can_messages::GetSpeedRequest &m, FDCAN_HandleTypeDef *can_handle) {
        TxHeader.Identifier = static_cast<uint32_t >(m.id);
        TxHeader.IdType = FDCAN_EXTENDED_ID;
        TxHeader.TxFrameType = FDCAN_DATA_FRAME;
        TxHeader.DataLength = FDCAN_DLC_BYTES_4;
        TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
        TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
        TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
        TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        TxHeader.MessageMarker = 0;

        auto response = can_messages::GetSpeedResponse{speed};

        auto sp = BodyType(buff);
        response.serialize(sp);

        HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data());
    }

    void handle(const std::monostate &m, FDCAN_HandleTypeDef *can_handle) { static_cast<void>(m); }

    can_parse::Parser<MessageTypes...> parser;

    uint32_t speed{0};
};

static auto message_handler = MessageHandler<can_messages::MoveRequest, can_messages::SetSpeedRequest,
    can_messages::SetupRequest, can_messages::GetStatusRequest, can_messages::GetSpeedRequest>{};



static void run(void *parameter) {
    parameter = nullptr;

    if (MX_FDCAN1_Init(&can) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&can) != HAL_OK) {
        Error_Handler();
    }

    TxHeader.IdType = FDCAN_EXTENDED_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    for (;;) {
        if (HAL_FDCAN_GetRxFifoFillLevel(&can, FDCAN_RX_FIFO0) > 0) {
            // Send CAN RX data over debug UART
            HAL_FDCAN_GetRxMessage(&can, FDCAN_RX_FIFO0, &RxHeader,
                                   buff.data());

            BodyType bod{buff};
            message_handler.handle(&can, static_cast<can_ids::MessageId>(RxHeader.Identifier), bod);
//
//            TxHeader.Identifier = RxHeader.Identifier;
//            TxHeader.DataLength = RxHeader.DataLength;
//            TxHeader.FDFormat = RxHeader.FDFormat;
//
//            if (HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data()) !=
//                HAL_OK) {
//                // Transmission request Error
//                Error_Handler();
//            }
        }
        vTaskDelay(1);
    }
}

static constexpr auto stack_size = 300;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::array<StackType_t, stack_size> stack;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static StaticTask_t data{};


/**
 * Create the task.
 */
void can_task::start() {
    xTaskCreateStatic(run, "CAN Task", stack.size(), nullptr, 1, stack.data(),
                      &data);
}

void Error_Handler() {
    for (;;) {
        HAL_GPIO_TogglePin(
            GPIOA,         // NOLINTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            GPIO_PIN_5);   // NOLINTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        vTaskDelay(1000);  // NOLINTLINE(cppcoreguidelines-avoid-magic-numbers)
    }
}