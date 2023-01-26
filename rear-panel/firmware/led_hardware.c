/**PB11 DECK_LED DRIVE 
	AF1 = TIM2_CH4
//PC6 BLUE DRIVE 
	AF2 = TIM3_CH1
	AF4 = TIM8_CH1
//PC7 WHITE DRIVE
	AF2 = TIM3_CH2
	AF4 = TIM8_CH2
//PB14 GREEN Drive
	AF1 = TIM15_CH1
//PB15 RED Drive
	AF2 = TIM15_CH2
**/
#include "rear-panel/firmware/led_hardware.h"
#include "common/firmware/errors.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim15;

TIM_OC_InitTypeDef htim2_sConfigOC = {0};
TIM_OC_InitTypeDef htim3_sConfigOC = {0};
TIM_OC_InitTypeDef htim15_sConfigOC = {0};


uint32_t round_closest(uint32_t dividend, uint32_t divisor) {
    return (dividend + (divisor / 2)) / divisor;
}

uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq) {
    return timer_clk_freq >= counter_clk_freq
               ? round_closest(timer_clk_freq, counter_clk_freq) - 1U
               : 0U;
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim->Instance == TIM2) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM2 GPIO Configuration
        PB11     ------> TIM2_CH4
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    } else if (htim->Instance == TIM3) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PC6     ------> TIM3_CH1
        PC7     ------> TIM3_CH2
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    } else if (htim->Instance == TIM15) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PB14     ------> TIM15_CH1
        PB15     ------> TIM15_CH2
        */
        GPIO_InitStruct.Pin = GPIO_PIN_14;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM15;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM15;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}
 

/**
 * @brief TIM2 Initialization Function for DECK LED
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim2.Instance = TIM2;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim2.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = LED_PWM_WIDTH - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.RepetitionCounter = 0;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim2_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim2_sConfigOC.Pulse = 0;
    htim2_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim2_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim2_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim2_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim2_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &htim2_sConfigOC, TIM_CHANNEL_4) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim2, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim2);
}


/**
 * @brief TIM3 Initialization Function for BLUE and WHITE UI ligths
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim3.Instance = TIM3;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim3.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = LED_PWM_WIDTH - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0;
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
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim3_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim3_sConfigOC.Pulse = 0;
    htim3_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim3_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim3_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim3_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim3_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &htim3_sConfigOC, TIM_CHANNEL_1) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &htim3_sConfigOC, TIM_CHANNEL_2) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim3, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim3);
}


/**
 * @brief TIM15 Initialization Function for BLUE and GREEN UI ligths
 * @param None
 * @retval None
 */
static void MX_TIM15_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim15.Instance = TIM15;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim15.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim15.Init.Period = LED_PWM_WIDTH - 1;
    htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim15.Init.RepetitionCounter = 0;
    htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim15) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim15) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim15_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim15_sConfigOC.Pulse = 0;
    htim15_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim15_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim15_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim15_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim15_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim15, &htim15_sConfigOC, TIM_CHANNEL_1) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim15, &htim15_sConfigOC, TIM_CHANNEL_2) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim15);
}


void update_pwm(uint32_t duty_cycle, LED_DEVICE led) {
	
	switch(led) {
		case DECK_LED:
			htim2.Instance->CCR4 = duty_cycle;
			break;
		case BLUE_UI_LED:
			htim3.Instance->CCR1 = duty_cycle;
			break;
		case WHITE_UI_LED:
			htim3.Instance->CCR2 = duty_cycle;
			break;
		case GREEN_UI_LED:
			htim15.Instance->CCR1=duty_cycle;
			break;
		case RED_UI_LED:
			htim15.Instance->CCR2=duty_cycle;
			break;
	}
}

void initialize_leds() {
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM15_Init();
}
