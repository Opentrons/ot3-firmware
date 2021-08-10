#include "common/firmware/can_task.hpp"

#include <array>

#include "FreeRTOS.h"
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

            TxHeader.Identifier = RxHeader.Identifier;
            TxHeader.DataLength = RxHeader.DataLength;
            TxHeader.FDFormat = RxHeader.FDFormat;

            if (HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff.data()) !=
                HAL_OK) {
                // Transmission request Error
                Error_Handler();
            }
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