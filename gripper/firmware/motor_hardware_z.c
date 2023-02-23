#include "motor_hardware.h"

TIM_HandleTypeDef htim7;


/** Simple time base for Z motor interrupt **/
static void TIM7_Base_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim7.State = HAL_TIM_STATE_RESET;
    htim7.Instance = TIM7;
    /*
     * Setting counter clock frequency to 100 kHz
     */
    htim7.Init.Prescaler = 425;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 1;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
}


/** SPI for configuring Z motor driver **/
SPI_HandleTypeDef hspi2 = {
    .Instance = SPI2,
    .State = HAL_SPI_STATE_RESET,
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

/**
* @brief  Initialize the SPI MSP.
* @param  hspi pointer to a SPI_HandleTypeDef structure that contains
*         the configuration information for SPI module.
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI2) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        /* SPI2 GPIO Configuration
        PB12  ------> SPI2_CS
        PB13  ------> SPI2_SCK
        PB14  ------> SPI2_MISO
        PB15  ------> SPI2_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

/**
 * @brief SPI MSP De-Initialization
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                   GPIO_PIN_15);
    }
}


HAL_StatusTypeDef initialize_spi() {
    __HAL_RCC_SPI2_CLK_ENABLE();
    return HAL_SPI_Init(&hspi2);
}


void initialize_hardware_z() {
    TIM7_Base_Init();
}
