#include "motor_hardware.h"
#include "stm32g4xx_hal.h"
#include "common/firmware/errors.h"

TIM_HandleTypeDef htim7;
motor_interrupt_callback motor_callback = NULL;

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI
         Step/Dir
         PC7  ---> Dir Pin Motor A
         PC6  ---> Step Pin Motor A
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip select and tmc2130 clock and enable
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_0 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);

        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }

    else if (hspi->Instance == SPI3) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SPI3 GPIO Configuration
        PA4     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI
         Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Ctrl/Dir/Step pin for motor on SPI3 (Z-axis)
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);
    }
}

/**
 * @brief SPI MSP De-Initialization
 * This function denits SPI for Z/A motors
 * @param hspi: SPI2 handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI2 GPIO Configuration
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB14     ------> SPI2_MISO
        PB15     ------> SPI2_MOSI

        Step/Dir
         PC7  ---> Dir Pin Motor on SPI2 (A-axis)
         PC6  ---> Step Pin Motor on SPI2 (A-axis)
        */
        HAL_GPIO_DeInit(GPIOB,
                        GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
    } else if (hspi->Instance == SPI3) {
        /* Peripheral clock disable */
        __HAL_RCC_SPI2_CLK_DISABLE();

        /**SPI3 GPIO Configuration
        PA04     ------> SPI3_NSS
        PC10     ------> SPI3_SCK
        PC11     ------> SPI3_MISO
        PC12     ------> SPI3_MOSI
        Step/Dir
         PC1  ---> Dir Pin Motor on SPI3 (Z-axis)
         PC0  ---> Step Pin Motor on SPI3 (Z-axis)
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
                                   GPIO_PIN_0 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
    }
}
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

SPI_HandleTypeDef hspi3 = {
    .Instance = SPI3,
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

HAL_StatusTypeDef initialize_spi(SPI_HandleTypeDef* hspi) {
    if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
    } else {
        return HAL_ERROR;
    }
    return HAL_SPI_Init(hspi);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void MX_GPIO_Init(void) {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 849;
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Check which version of the timer triggered this callback
    if ((htim == &htim7) && motor_callback) {
        motor_callback();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim7) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();

        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
}

void initialize_timer(motor_interrupt_callback callback) {
    motor_callback = callback;
    MX_GPIO_Init();
    MX_TIM7_Init();
}
