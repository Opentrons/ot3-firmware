#include "hepa-uv/firmware/hardware.h"
#include "hepa-uv/firmware/utility_gpio.h"
#include "common/firmware/errors.h"

#include "platform_specific_hal_conf.h"
#include "system_stm32g4xx.h"

TIM_HandleTypeDef htim3;

TIM_OC_InitTypeDef htim3_sConfigOC = {0};


// void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_pwm) {
//     if(htim_pwm->Instance == TIM3) {
//         __HAL_RCC_TIM3_CLK_ENABLE();
//     }
// }

// void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};
//     if (htim->Instance == TIM3) {
//         __HAL_RCC_GPIOA_CLK_ENABLE();
//         /**TIM3 GPIO Configuration
//         PA6     ------> TIM3_CH1
//         */
//         GPIO_InitStruct.Pin = HEPA_PWM_PIN;
//         GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//         GPIO_InitStruct.Pull = GPIO_NOPULL;
//         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//         GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
//         HAL_GPIO_Init(HEPA_PWM_PORT, &GPIO_InitStruct);
//     }
// }

/**
 * @brief TIM3 Initialization Function for HEPA_PWM
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim3.State = HAL_TIM_STATE_RESET;
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 64 - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 40 - 1;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0;
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
    htim3_sConfigOC.OCMode = TIM_OCMODE_PWM1;
    // 0% duty cycle
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
    // HAL_TIM_MspPostInit(&htim3);
}

void hepa_fan_hw_update_pwm(uint32_t duty_cycle) {
    // update hepa fan speed
    htim3.Instance->CCR1 = duty_cycle;
}

void initialize_hardware() {
    MX_TIM3_Init();

    // Set the hepa fan speed to 0
    hepa_fan_hw_update_pwm(0);

    // Activate the channels
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
