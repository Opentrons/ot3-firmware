#include "hepa-uv/firmware/hepa_hardware.h"
#include "hepa-uv/firmware/utility_gpio.h"
#include "common/firmware/errors.h"
#include "timer_hardware.h"

TIM_HandleTypeDef htim2;
TIM_MasterConfigTypeDef sMasterConfig = {0};
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
    htim2.Init.Prescaler = 2;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4.294967295E9;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    /* Initialize Input Capture mode */
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* Initialize TIM2 input capture channel */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV4;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    /* The update event of the enable timer is interrupted */
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    /* Start the input capture measurement */
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    // /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
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

uint32_t val1 = 0;
uint32_t val2 = 0;
uint32_t tach_period = 0;
uint8_t captured_val1 = 0;

uint32_t get_hepa_fan_rpm(void) {
    if (tach_period == 0) return 0;
    uint32_t rpm = ((uint32_t)HEPA_TACHOMETER_TIMER_FREQ * 60) \
        / ((uint32_t)tach_period);
    return rpm;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        uint32_t count = __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)htim);
        if (captured_val1 == 0) {
            val1 = count;
            captured_val1 = 1;
        } else {
            val2 = count;
        }

        if (val2 > val1) {
            tach_period = val2 - val1;
            captured_val1 = 0;
            __HAL_TIM_SET_COUNTER(htim, 0);
        } else {
            tach_period = 0;
        }

        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    }
}

void initialize_tachometer() {
    tachometer_gpio_init();
    TIM2_Tachometer_Init();
}