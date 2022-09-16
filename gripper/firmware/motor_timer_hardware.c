#include "motor_encoder_hardware.h"
#include "motor_hardware.h"
#include "system_stm32g4xx.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim7;

TIM_OC_InitTypeDef htim1_sConfigOC = {0};
TIM_OC_InitTypeDef htim3_sConfigOC = {0};

static motor_interrupt_callback timer_callback = NULL;
static brushed_motor_interrupt_callback brushed_timer_callback = NULL;
static encoder_overflow_callback gripper_enc_overflow_callback = NULL;
static encoder_idle_state_callback gripper_enc_idle_state_overflow_callback =
    NULL;

uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

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
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    /*
     * Setting counter clock frequency to 32 kHz
     * Note that brushed timer tick at a different frequency from the stepper
     * motor timer.
     */
    htim1.Init.Prescaler =
        calc_prescaler(SystemCoreClock, GRIPPER_JAW_TIMER_FREQ);
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = GRIPPER_JAW_PWM_WIDTH - 1;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
    htim1_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 66% */
    htim1_sConfigOC.Pulse = GRIPPER_JAW_PWM_WIDTH * 2 / 3;
    htim1_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim1_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim1_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim1_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim1_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &htim1_sConfigOC, TIM_CHANNEL_1) !=
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

    htim3.Instance = TIM3;
    /*
     * Setting counter clock frequency to 32 kHz
     * Note that brushed timer tick at a different frequency from the stepper
     * motor timer.
     */
    htim3.Init.Prescaler =
        calc_prescaler(SystemCoreClock, GRIPPER_JAW_TIMER_FREQ);
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = GRIPPER_JAW_PWM_WIDTH - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
    htim3_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 66% */
    htim3_sConfigOC.Pulse = GRIPPER_JAW_PWM_WIDTH * 2 / 3;
    htim3_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim3_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &htim3_sConfigOC, TIM_CHANNEL_1) !=
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
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
    } else if (htim_base->Instance == TIM3) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /* TIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    } else if (htim_base->Instance == TIM4) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM4_CLK_DISABLE();
    }
}

void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    /*
     * Setting counter clock frequency to 100 kHz
     */
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
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    } else if (htim == &htim3) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    } else if (htim == &htim4) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM4_CLK_ENABLE();
        /* TIM4 interrupt Init */
        HAL_NVIC_SetPriority(TIM4_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
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

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC,  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  &GPIO_InitStruct);
}

void initialize_timer(motor_interrupt_callback callback) {
    timer_callback = callback;
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM3_Init();
    MX_TIM7_Init();
}

void update_pwm(uint32_t duty_cycle) {
    // we allow period + 1 here because that forces an always-high duty
    // (in pwm mode 1, output is high while cnt < ccr, so we don't want
    // to let cnt = ccr if the pwm value is 100%)
    htim1.Instance->CCR1 = clamp(duty_cycle, 0, htim1.Init.Period + 1);

    htim3.Instance->CCR1 = clamp(duty_cycle, 0, htim3.Init.Period + 1);
}

void set_brushed_motor_timer_callback(
    brushed_motor_interrupt_callback callback,
    encoder_overflow_callback g_enc_f_callback,
    encoder_idle_state_callback g_enc_idle_callback) {
    brushed_timer_callback = callback;
    gripper_enc_overflow_callback = g_enc_f_callback;
    gripper_enc_idle_state_overflow_callback = g_enc_idle_callback;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    // Check which version of the timer triggered this callback
    if (htim == &htim7 && timer_callback) {
        timer_callback();

    } else if (htim == &htim1 && brushed_timer_callback) {
        brushed_timer_callback();

    } else if (htim == &htim2 && gripper_enc_overflow_callback) {
        uint32_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(htim);
        gripper_enc_overflow_callback(direction ? -1 : 1);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);

    } else if (htim == &htim4 && gripper_enc_idle_state_overflow_callback) {
        gripper_enc_idle_state_overflow_callback(true);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {
    if (htim == &htim4 && gripper_enc_idle_state_overflow_callback) {
        gripper_enc_idle_state_overflow_callback(false);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1);
    }
}