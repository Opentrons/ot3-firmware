#include "hepa-uv/firmware/led_hardware.h"
#include "hepa-uv/firmware/utility_gpio.h"
#include "common/firmware/errors.h"

#include "platform_specific_hal_conf.h"
#include "system_stm32g4xx.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim20;

TIM_OC_InitTypeDef htim1_sConfigOC = {0};
TIM_OC_InitTypeDef htim8_sConfigOC = {0};
TIM_OC_InitTypeDef htim16_sConfigOC = {0};
TIM_OC_InitTypeDef htim20_sConfigOC = {0};


uint32_t round_closest(uint32_t dividend, uint32_t divisor) {
    return (dividend + (divisor / 2)) / divisor;
}

uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq) {
    return timer_clk_freq >= counter_clk_freq
               ? round_closest(timer_clk_freq, counter_clk_freq) - 1U
               : 0U;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_pwm) {
    if(htim_pwm->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();
    } else if(htim_pwm->Instance == TIM8) {
        __HAL_RCC_TIM8_CLK_ENABLE();
    } else if(htim_pwm->Instance == TIM16) {
        __HAL_RCC_TIM16_CLK_ENABLE();
    } else if(htim_pwm->Instance == TIM20) {
        __HAL_RCC_TIM20_CLK_ENABLE();
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim->Instance == TIM1) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PA9     ------> TIM1_CH2
        PA10    ------> TIM1_CH3
        */
        GPIO_InitStruct.Pin = HEPA_R_CTRL_PIN | HEPA_W_CTRL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;
        HAL_GPIO_Init(HEPA_R_CTRL_PORT, &GPIO_InitStruct);

        // PC5     ------> TIM1_CH4N
        GPIO_InitStruct.Pin = UV_B_CTRL_PIN;
        HAL_GPIO_Init(UV_B_CTRL_PORT, &GPIO_InitStruct);
    } else if (htim->Instance == TIM8) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM8 GPIO Configuration
        PB0     ------> TIM8_CH2N
        PB1     ------> TIM8_CH3N
        */
        GPIO_InitStruct.Pin = UV_G_CTRL_PIN | UV_R_CTRL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_TIM8;
        HAL_GPIO_Init(UV_G_CTRL_PORT, &GPIO_InitStruct);

        /*
        PC6     ------> TIM8_CH1
        */
        GPIO_InitStruct.Pin = HEPA_G_CTRL_PIN;
        HAL_GPIO_Init(HEPA_G_CTRL_PORT, &GPIO_InitStruct);
    } else if (htim->Instance == TIM16) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM16 GPIO Configuration
        PB4     ------> TIM16_CH1
        */
        GPIO_InitStruct.Pin = HEPA_B_CTRL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM16;
        HAL_GPIO_Init(HEPA_B_CTRL_PORT, &GPIO_InitStruct);
    } else if (htim->Instance == TIM20) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM20 GPIO Configuration
        PB2     ------> TIM20_CH1
        */
        GPIO_InitStruct.Pin = UV_W_CTRL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM20;
        HAL_GPIO_Init(UV_W_CTRL_PORT, &GPIO_InitStruct);
    }
}

/**
 * @brief TIM1 Initialization Function for HEPA_R_CTRL, HEPA_W_CTRL, and UV_B_CTRL
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.State = HAL_TIM_STATE_RESET;
    htim1.Instance = TIM1;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim1.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = LED_PWM_WIDTH - 1;
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
    htim1_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim1_sConfigOC.Pulse = 0;
    htim1_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim1_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim1_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim1_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim1_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &htim1_sConfigOC, TIM_CHANNEL_2) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &htim1_sConfigOC, TIM_CHANNEL_3) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &htim1_sConfigOC, TIM_CHANNEL_4) !=
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
 * @brief TIM8 Initialization Function for HEPA_G_CTRL, UV_G_CTRL, and UV_R_CTRL
 * @param None
 * @retval None
 */
static void MX_TIM8_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim8.State = HAL_TIM_STATE_RESET;
    htim8.Instance = TIM8;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim8.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = LED_PWM_WIDTH - 1;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim8) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim8_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim8_sConfigOC.Pulse = 0;
    htim8_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim8_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim8_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim8_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim8_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim8, &htim8_sConfigOC, TIM_CHANNEL_1) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim8, &htim8_sConfigOC, TIM_CHANNEL_2) !=
        HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim8, &htim8_sConfigOC, TIM_CHANNEL_3) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim8);
}

/**
 * @brief TIM16 Initialization Function for HEPA_B_CTRL
 * @param None
 * @retval None
 */
static void MX_TIM16_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim16.State = HAL_TIM_STATE_RESET;
    htim16.Instance = TIM16;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim16.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim16.Init.Period = LED_PWM_WIDTH - 1;
    htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim16.Init.RepetitionCounter = 0;
    htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim16) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim16, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim16) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim16, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim16_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim16_sConfigOC.Pulse = 0;
    htim16_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim16_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim16_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim16_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim16_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim16, &htim16_sConfigOC, TIM_CHANNEL_1) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim16);
}

/**
 * @brief TIM20 Initialization Function for UV_W_CTRL
 * @param None
 * @retval None
 */
static void MX_TIM20_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim20.State = HAL_TIM_STATE_RESET;
    htim20.Instance = TIM20;
    /*
     * Setting counter clock frequency to 2 kHz
     */
    htim20.Init.Prescaler =
        calc_prescaler(SystemCoreClock, LED_TIMER_FREQ);
    htim20.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim20.Init.Period = LED_PWM_WIDTH - 1;
    htim20.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim20.Init.RepetitionCounter = 0;
    htim20.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim20) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim20, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim20) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim20, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    htim20_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    /* Set duty cycle at 0% */
    htim20_sConfigOC.Pulse = 0;
    htim20_sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    htim20_sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    htim20_sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    htim20_sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    htim20_sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim20, &htim20_sConfigOC, TIM_CHANNEL_1) !=
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
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim20, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    HAL_TIM_MspPostInit(&htim20);
}

void button_led_hw_update_pwm(uint32_t duty_cycle, LED_TYPE led, PUSH_BUTTON_TYPE button) {

    // TODO: fix this
    if (button == HEPA_BUTTON) {
        switch(led) {
            case RED_LED:
                htim1.Instance->CCR2 = duty_cycle;
                break;
            case GREEN_LED:
                htim8.Instance->CCR1 = duty_cycle;
                break;
            case BLUE_LED:
                htim16.Instance->CCR1 = duty_cycle;
                break;
            case WHITE_LED:
                htim1.Instance->CCR3 = duty_cycle;
                break;
            default:
                break;
        }
    } else if (button == UV_BUTTON) {
        switch(led) {
            case RED_LED:
                htim8.Instance->CCR3 = duty_cycle;
                break;
            case GREEN_LED:
                htim8.Instance->CCR2 = duty_cycle;
                break;
            case BLUE_LED:
                htim1.Instance->CCR4 = duty_cycle;
                break;
            case WHITE_LED:
                htim20.Instance->CCR1=duty_cycle;
                break;
            default:
                break;
        }
    }
}

void set_button_led_pwm(PUSH_BUTTON_TYPE button, uint32_t red, uint32_t green, uint32_t blue, uint32_t white) {
    button_led_hw_update_pwm(red, RED_LED, button);
    button_led_hw_update_pwm(green, GREEN_LED, button);
    button_led_hw_update_pwm(blue, BLUE_LED, button);
    button_led_hw_update_pwm(white, WHITE_LED, button);
}

void button_hw_initialize_leds() {
    MX_TIM1_Init();
    MX_TIM8_Init();
    MX_TIM16_Init();
    MX_TIM20_Init();

    // Set the the button LEDS to idle (white)
    set_button_led_pwm(HEPA_BUTTON, 0, 0, 0, 50);
    set_button_led_pwm(UV_BUTTON, 0, 0, 0, 50);

    // Activate the channels
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim20, TIM_CHANNEL_1);
    // Activate the complementary output
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);
}
