#include "motor_hardware.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim7;

static motor_interrupt_callback timer_callback = NULL;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim->Instance == TIM1) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PC0     ------> TIM1_CH1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    } else if (htim->Instance == TIM3) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PA6     ------> TIM3_CH1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
 * @brief TIM1 Initialization Function for AIN1
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 50000;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 4000;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 2000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) !=
        HAL_OK) {
        Error_Handler();
    }
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim1);
}

/**
 * @brief TIM3 Initialization Function for AIN2
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 50000;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 4000;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 2000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim3);
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base) {
    if (htim_base->Instance == TIM1) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();
        /* TIM1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
    } else if (htim_base->Instance == TIM3) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /* TIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    // Check which version of the timer triggered this callback
    if (htim == &htim7 && timer_callback) {
        timer_callback();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim) {
    if (htim == &htim7) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();
        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    } else if (htim == &htim1) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
        /* TIM1 interrupt Init */
        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
    } else if (htim == &htim3) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
}

void initialize_timer(motor_interrupt_callback callback) {
    timer_callback = callback;
    MX_GPIO_Init();
    MX_TIM7_Init();
}

void initialize_pwm() {
    MX_TIM1_Init();
    MX_TIM3_Init();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}
