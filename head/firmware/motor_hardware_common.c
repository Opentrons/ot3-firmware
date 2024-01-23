#include "common/firmware/errors.h"
#include "motor_hardware.h"
#include "stm32g4xx_hal.h"

TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim2 = {
    .Instance = TIM2,
    .Init = {.Prescaler = 0,
             .CounterMode = TIM_COUNTERMODE_UP,
             .Period = UINT16_MAX,
             .ClockDivision = TIM_CLOCKDIVISION_DIV1,
             .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE}};
TIM_HandleTypeDef htim3 = {
    .Instance = TIM3,
    .Init = {.Prescaler = 0,
             .CounterMode = TIM_COUNTERMODE_UP,
             .Period = UINT16_MAX,
             .ClockDivision = TIM_CLOCKDIVISION_DIV1,
             .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE}};

motor_interrupt_callback motor_callback = NULL;
encoder_overflow_callback left_enc_overflow_callback = NULL;
encoder_overflow_callback right_enc_overflow_callback = NULL;

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
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

        // Chip select
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

        // A motor step and direction
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
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
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

        // Dir/Step pin for motor on SPI3 (Z-axis)
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
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
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
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

HAL_StatusTypeDef initialize_spi(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
    } else if (hspi->Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
    } else {
        return HAL_ERROR;
    }
    return HAL_SPI_Init(hspi);
}

void Encoder_GPIO_Init(void) {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    /* Encoder A Axis GPIO Configuration
    PA0     ------> CHANNEL A
    PA1     ------> CHANNEL B
    PA5    ------> CHANNEL I (UNUSED)
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Encoder Z Axis GPIO Configuration
    PA6     ------> CHANNEL A
    PA7     ------> CHANNEL B
    PD2    ------> CHANNEL I (UNUSED)
    */
    // ENC CHANNELA Z AXIS PIN Configure
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
    initialize_rev_specific_pins();
}

#if PCBA_PRIMARY_REVISION == 'b' || PCBA_PRIMARY_REVISION == 'a'
    static const int encoder_chan1_direction = TIM_ICPOLARITY_RISING;
#else
    static const int encoder_chan1_direction = TIM_ICPOLARITY_FALLING;
#endif


void encoder_init(TIM_HandleTypeDef *htim) {
    TIM_Encoder_InitTypeDef sConfig = {
        .EncoderMode = TIM_ENCODERMODE_TI12,
        .IC1Polarity = encoder_chan1_direction,
        .IC1Selection = TIM_ICSELECTION_DIRECTTI,
        .IC1Prescaler = TIM_ICPSC_DIV1,
        .IC1Filter = 0,
        .IC2Polarity = TIM_ICPOLARITY_RISING,
        .IC2Selection = TIM_ICSELECTION_DIRECTTI,
        .IC2Prescaler = TIM_ICPSC_DIV1,
        .IC2Filter = 0,
    };
    if (HAL_TIM_Encoder_Init(htim, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_RESET,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE};
    if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }
    /* Reset counter */
    __HAL_TIM_SET_COUNTER(htim, 0);
    /* Clear interrupt flag bit */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    /* The update event of the enable timer is interrupted */
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    /* Set update event request source as: counter overflow */
    __HAL_TIM_URS_ENABLE(htim);
    /* Enable UIFREMAP so the MSb of the count register reflects overflows */
    __HAL_TIM_UIFREMAP_ENABLE(htim);
    /* Enable encoder interface */
    HAL_TIM_Encoder_Start_IT(htim, TIM_CHANNEL_ALL);
}

// motor timer: 200kHz from
// 170MHz sysclk
// /1 AHB
// /2 APB1
// /425 prescaler = 200kHz
void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Check which version of the timer triggered this callback
    if ((htim == &htim7) && motor_callback) {
        motor_callback();
    } else if (htim == &htim2 && right_enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        right_enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    } else if (htim == &htim3 && left_enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        left_enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
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

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
        /* TIM2 interrupt Init */
    } else if (htim == &htim3) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // disengage motor whenever estop is engaged
    if (GPIO_Pin == GPIO_PIN_4) {
        #if PCBA_PRIMARY_REVISION != 'b' && PCBA_PRIMARY_REVISION != 'a'
            // right & left brake
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_5, GPIO_PIN_RESET);
        #endif

        // disable both left and right enable pins
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    }
}

void initialize_timer(motor_interrupt_callback callback,
                      encoder_overflow_callback l_f_callback,
                      encoder_overflow_callback r_f_callback) {
    motor_callback = callback;
    left_enc_overflow_callback = l_f_callback;
    right_enc_overflow_callback = r_f_callback;
    MX_GPIO_Init();
    Encoder_GPIO_Init();
    encoder_init(&htim2);
    encoder_init(&htim3);
    MX_TIM7_Init();
}
