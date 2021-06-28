#include <cstdio>

#include "pipettes/firmware/uart.hpp"

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

UART_HandleTypeDef MX_USART1_UART_Init(UART_HandleTypeDef huart1) {
    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    constexpr auto baud_rate = 9600;
    huart1.Instance =
        LPUART1;  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    huart1.Init.BaudRate = baud_rate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
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
    return huart1;
}

void Error_Handler() { puts("We encountered an error"); }
