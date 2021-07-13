#include "pipettes/firmware/can_task.hpp"

#include <array>
#include "FreeRTOS.h"
#include "common/firmware/can.h"
#include "task.h"


static void Error_Handler();

FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_HandleTypeDef can{};

static void run(void *parameter) {
    parameter = nullptr;

    Error_Handler();

    auto result = MX_FDCAN1_Init(&can);
    if (result != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&can) != HAL_OK)
    {
        Error_Handler();
    }

    TxHeader.Identifier = 0x555;
    TxHeader.IdType = FDCAN_EXTENDED_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
    uint8_t buff[] = {1, 2, 3, 4, 5, 6, 7, 8};

    for (;;) {
        if (HAL_FDCAN_AddMessageToTxFifoQ(&can, &TxHeader, buff) != HAL_OK)
        {
            // Transmission request Error
            Error_Handler();
        }
        vTaskDelay(1000);
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
    for(;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        vTaskDelay(1000);
    }
}