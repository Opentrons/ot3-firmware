#include "motor_hardware.h"

#include "common/firmware/errors.h"
#include "stm32g4xx_hal.h"

DAC_HandleTypeDef hdac1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

TIM_OC_InitTypeDef htim1_sConfigOC = {0};
TIM_OC_InitTypeDef htim3_sConfigOC = {0};


static uint32_t round_closest(uint32_t dividend, uint32_t divisor) {
    return (dividend + (divisor / 2)) / divisor;
}

static uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq) {
    return timer_clk_freq >= counter_clk_freq
               ? round_closest(timer_clk_freq, counter_clk_freq) - 1U
               : 0U;
}

static uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}


void update_pwm(uint32_t duty_cycle) {
    // we allow period + 1 here because that forces an always-high duty
    // (in pwm mode 1, output is high while cnt < ccr, so we don't want
    // to let cnt = ccr if the pwm value is 100%)
    htim1.Instance->CCR1 = clamp(duty_cycle, 0, htim1.Init.Period + 1);

    htim3.Instance->CCR1 = clamp(duty_cycle, 0, htim3.Init.Period + 1);
}

static void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim->Instance == TIM1) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /** TIM1 GPIO Configuration
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
        /** TIM3 GPIO Configuration
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
static void TIM1_PWM_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    htim1.State = HAL_TIM_STATE_RESET;
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
    // lower level resources are set after init
    HAL_TIM_MspPostInit(&htim1);
}

/**
 * @brief TIM3 Initialization Function for AIN2
 * @param None
 * @retval None
 */
static void TIM3_PWM_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.State = HAL_TIM_STATE_RESET;
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
    // lower level resources are set after init
    HAL_TIM_MspPostInit(&htim3);
}



#if PCBA_PRIMARY_REVISION == 'a'
uint32_t ic2_polarity = TIM_ICPOLARITY_RISING;
#else
uint32_t ic2_polarity = TIM_ICPOLARITY_FALLING;
#endif

static void TIM2_EncoderG_Init(void) {
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.State = HAL_TIM_STATE_RESET;
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
    sConfig.IC2Polarity = ic2_polarity;
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
    /* Enable encoder interface */
    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
}

/**
 * TIM4 is used to measure the timelapse between two consecutive edges
 * of the encoder clock. The value can then be used to estimate the speed at
 * which the encoder is rotating, hence, the speed at which the gripper jaw is
 * travelling.
 *
 * TIM4 receives the encoder clock output signal from TIM2 (tim_trgo). It counts
 * at 1 MHz and captures the current count whenever a full encoder tick
 * (4 pulses)  is received, then resets its counter.
 * The physical movement is 0.1637896203 um per pulse so is 0.655158481 um
 * per full tick, so the velocity is ~(0.6552/count) um/us or ~(655.2/count) mm/s
 * TIM2 is counting both rising and falling edges from channel A + B.
 * so this timer uses a DIV4 prescaller to count only complete ticks
 *
 * The timer overflows 3 ms after an tick is received. This means any encoder
 * movement below 333Hz, which is about 0.2184 mm/s
 * is rejected and that we can safely assume the encoder has stopped moving.
 **/
static void TIM4_EncoderGSpeed_Init(void) {
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    htim4.Instance = TIM4;
    htim4.State = HAL_TIM_STATE_RESET;
    htim4.Init.Prescaler =
        calc_prescaler(SystemCoreClock, GRIPPER_ENCODER_SPEED_TIMER_FREQ);
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 3000;  // timer overflows after 3 ms
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    /* Initialize Input Capture mode */
    if (HAL_TIM_IC_Init(&htim4) != HAL_OK) {
        Error_Handler();
    }
    /* Configure TIM4 in Slave mode */
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger =
        TIM_TS_ITR1;  // input trigger uses encoder clock
    if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* Initialize TIM4 input capture channel */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_TRC;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV4;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    /*
     * Only counter overflow/underflow generates an update interrupt.
     * This makes sure the input capture (when the encoder is moving) does not
     * trigger an update event!
     */
    __HAL_TIM_URS_ENABLE(&htim4);
    /* The update event of the enable timer is interrupted */
    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
    /* Start the input capture measurement */
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
}

/**
 * @brief DAC1 Initialization Function
 * @param None
 * @retval None
 */
static void DAC1_Init(void) {
    hdac1.Instance = DAC1;
    hdac1.State = HAL_DAC_STATE_RESET;
    if (HAL_DAC_Init(&hdac1) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief DAC MSP Initialization
 * @param hdac: DAC handle pointer
 * @retval None
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hdac->Instance == DAC1) {
        /* Peripheral clock enable */
        __HAL_RCC_DAC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**DAC1 GPIO Configuration
        PA4     ------> DAC1_OUT1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
 * @brief DAC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hdac: DAC handle pointer
 * @retval None
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac) {
    if (hdac->Instance == DAC1) {
        __HAL_RCC_DAC1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
    }
}

void initialize_hardware_g() {
    TIM1_PWM_Init();
    TIM3_PWM_Init();
    TIM2_EncoderG_Init();
    TIM4_EncoderGSpeed_Init();
    DAC1_Init();
}

