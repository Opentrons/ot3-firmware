#include "common/firmware/uart.h"

static void Error_Handler();
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit() {
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_SYSCFG_CLK_ENABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    __HAL_RCC_PWR_CLK_ENABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

    /* System interrupt init*/

    /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
     */
    HAL_PWREx_DisableUCPDDeadBattery();

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (huart->Instance ==
        LPUART1)  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_LPUART1_CLK_ENABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        __HAL_RCC_GPIOA_CLK_ENABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF12_LPUART1;
        HAL_GPIO_Init(
            GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            &GPIO_InitStruct);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
    if (huart->Instance ==
        LPUART1)  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    {
        /* USER CODE BEGIN USART1_MspDeInit 0 */
        __HAL_RCC_LPUART1_FORCE_RESET();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        __HAL_RCC_LPUART1_RELEASE_RESET();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPUART1_CLK_DISABLE();  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(
            GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            GPIO_PIN_2 |
                GPIO_PIN_3);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
}

UART_HandleTypeDef MX_LPUART1_UART_Init() {
    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    uint32_t baud_rate = 115200;
    UART_HandleTypeDef huart1 = {
        .Instance = LPUART1,
        .Init = {
            .BaudRate = baud_rate,
            .WordLength = UART_WORDLENGTH_8B,
            .StopBits = UART_STOPBITS_1,
            .Parity = UART_PARITY_NONE,
            .Mode = UART_MODE_TX_RX,
            .HwFlowCtl = UART_HWCONTROL_NONE,
            .OverSampling = UART_OVERSAMPLING_16,
            .OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE,
            .ClockPrescaler = UART_PRESCALER_DIV1
        },
        .AdvancedInit = {
            .AdvFeatureInit = UART_ADVFEATURE_NO_INIT
        }
    };

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
    return huart1;
}

void RCC_Peripheral_Clock_Select() {
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }

}

static void Error_Handler() {
    for(;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100);
    }
}
