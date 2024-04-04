#include "hepa-uv/firmware/hepa_hardware.h"
#include "hepa-uv/firmware/utility_gpio.h"
#include "common/firmware/errors.h"
#include "timer_hardware.h"
#include <math.h>

#include "stdio.h"

#define TIMCLOCK   85000000
#define PRESCALAR  85

TIM_HandleTypeDef htim2;
TIM_MasterConfigTypeDef sMasterConfig = {0};
TIM_ClockConfigTypeDef sClockSourceConfig = {0};
TIM_IC_InitTypeDef sConfigIC = {0};

void tachometer_gpio_init(void) {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    /* HEPA_FG GPIO Configuration
    PA5     ------> TIM2_CH1
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = HEPA_FG_MCU_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(HEPA_FG_MCU_PORT, &GPIO_InitStruct);
}

static void TIM2_Tachometer_Init(void) {
    htim2.Instance = TIM2;
    htim2.State = HAL_TIM_STATE_RESET;
    htim2.Init.Prescaler = PRESCALAR;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
        Error_Handler();

    /* Initialize Input Capture mode */
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) Error_Handler();
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* Initialize TIM2 input capture channel */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    // /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

static uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void set_hepa_fan_pwm(uint32_t duty_cycle) {
    // calculate the pulse width from the duty cycle
    uint32_t clamped_dc = clamp(duty_cycle, 0, 100) * htim3.Init.Period / 100;
    htim3.Instance->CCR1 = clamped_dc;
}

bool enable_hepa_fan_tachometer(bool enable) {
    /* The update event of the enable/disabled timer is interrupted */
    /* Start/Stop the input capture measurement */
    if (enable) {
        __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
        return HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) == HAL_OK;
    } else {
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
        return HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1) == HAL_OK;
    }
}

uint32_t captured_val = 0;
static hepa_rpm_callback hepa_rpm_cb = NULL;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
	{
	if (htim->Instance == TIM2)
	{
        if (captured_val == 0) {
            captured_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        } else {
            uint32_t new_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            uint32_t period = new_val > captured_val ? new_val - captured_val : ((0xffffffff - captured_val) + new_val) + 1;
            // reset and disable the timer
            captured_val = 0;
            enable_hepa_fan_tachometer(false);
            __HAL_TIM_SET_COUNTER(htim, 0);
            if (hepa_rpm_cb != NULL) {
                hepa_rpm_cb(round(60 / ((float)period / 1000000)));
            }
        }
	}
 }

void initialize_tachometer(hepa_rpm_callback rpm_callback) {
    hepa_rpm_cb = rpm_callback;
    tachometer_gpio_init();
    TIM2_Tachometer_Init();
}