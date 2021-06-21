#include <cstdio>
#include <cstring>

#include "FreeRTOS.h"
#include "STM32G491RETx/system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static UART_HandleTypeDef huart1;

static void Error_Handler();
static void MX_USART1_UART_Init();

void vTaskCode(void* vParamater) {
    vParamater = nullptr;
    MX_USART1_UART_Init();

    constexpr auto timeout = 0xFFFF;

    uint8_t c = 0;

    for (;;) {
        HAL_UART_Receive(&huart1, &c, 1, timeout);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Received %d", c);

        HAL_UART_Transmit(&huart1, &c, 1, timeout);
    }
}

auto main() -> int {
    HardwareInit();

    constexpr auto stack_size = 1000;

    xTaskCreate(vTaskCode, "USART Task", stack_size, nullptr, 0, nullptr);

    vTaskStartScheduler();

    return 0;
}

static void MX_USART1_UART_Init() {
    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    constexpr auto baud_rate = 115200;
    huart1.Instance = USART1;  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    huart1.Init.BaudRate = baud_rate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_ODD;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */
}

static void Error_Handler() { puts("We encountered an error"); }