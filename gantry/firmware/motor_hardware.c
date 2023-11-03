#include "motor_hardware.h"
#include "gantry/core/axis_type.h"


TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim2;

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI2) {
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB12  ------> SPI2_CS
        PB13  ------> SPI2_SCK
        PB14  ------> SPI2_MISO
        PB15  ------> SPI2_MOSI
        Step/Dir
        PB1   ------> Motor Dir Pin
        PC8   ------> Motor Step Pin
        Enable
        PA9   ------> Motor Enable Pin
        DIAG0
        PC5   ------> Motor Diag0 Pin
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Dir
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Step
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);

        // Enable
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOA,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                      &GPIO_InitStruct);

        // Diag0
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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
                                   GPIO_PIN_15 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_5);
    }
}

void gantry_driver_CLK_init(enum GantryAxisType gantry_axis) {
    switch (gantry_axis) {
        case gantry_x:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            // Driver Clock Pin
            GPIO_InitTypeDef GPIO_InitStruct = {0};
            GPIO_InitStruct.Pin = GPIO_PIN_5;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            break;
        case gantry_y:
            break;
        default:
            Error_Handler();
    }
}

HAL_StatusTypeDef initialize_spi(enum GantryAxisType gantry_type) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    gantry_driver_CLK_init(gantry_type);
    return HAL_SPI_Init(&hspi2);
}


static motor_interrupt_callback timer_callback = NULL;
static encoder_overflow_callback enc_overflow_callback = NULL;
static diag0_interrupt_callback* diag0_callback = NULL;


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

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// motor timer: 200kHz from
// 170MHz sysclk
// /1 AHB
// /2 APB1
// /425 prescaler = 200kHz
void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 424;
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

void Encoder_GPIO_Init(void) {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    /* Encoder GPIO Configuration
    PA0     ------> CHANNEL A
    PA1     ------> CHANNEL B
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM2_Encoder_Init(void) {
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = UINT16_MAX;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 0;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 0;
    if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENCODER_CLK;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* Reset counter */
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    /* Clear interrupt flag bit */
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    /* The update event of the enable timer is interrupted */
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    /* Set update event request source as: counter overflow */
    __HAL_TIM_URS_ENABLE(&htim2);
    /* Enable UIFREMAP so the MSb of the count register reflects overflows */
    __HAL_TIM_UIFREMAP_ENABLE(&htim2);
    /* Enable encoder interface */
    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Check which version of the timer triggered this callback
    if (htim == &htim7 && timer_callback) {
        timer_callback();
    } else if (htim == &htim2 && enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_5) {
        if (diag0_callback != NULL) {
            if (*diag0_callback != NULL) {
                (*diag0_callback)();
            }
        }
    }
}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
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

void initialize_timer(motor_interrupt_callback callback, diag0_interrupt_callback* diag0_int_callback, encoder_overflow_callback enc_callback) {
    timer_callback = callback;
    enc_overflow_callback = enc_callback;
    diag0_callback = diag0_int_callback;
    MX_GPIO_Init();
    MX_TIM7_Init();
    Encoder_GPIO_Init();
    TIM2_Encoder_Init();
}
