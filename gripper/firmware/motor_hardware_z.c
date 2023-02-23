#include "motor_hardware.h"

TIM_HandleTypeDef htim7;

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


SPI_HandleTypeDef hspi2 = {
    .Instance = SPI2,
    .State = HAL_TIM_STATE_RESET,
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
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                   GPIO_PIN_15 | GPIO_PIN_1 | GPIO_PIN_10);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
    }
}


HAL_StatusTypeDef initialize_spi() {
    __HAL_RCC_SPI2_CLK_ENABLE();

    return HAL_SPI_Init(&hspi2);
}


void initialize_hardware_z() {
    TIM7_Base_Init();
}
