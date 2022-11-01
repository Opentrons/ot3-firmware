#include "common/firmware/errors.h"
#include "stm32g4xx_hal_conf.h"

#include "motor_timer_hardware.h"
#include "pipettes/core/pipette_type.h"


TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim6;
static linear_motor_interrupt_callback plunger_callback = NULL;
static gear_motor_interrupt_callback gear_callback = NULL;

// motor timer: 200kHz from
// 170MHz sysclk
// /1 AHB
// /2 APB1
// /425 prescaler * 1 count = 200kHz
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

void MX_TIM6_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 499;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Check which version of the timer triggered this callback
    if ((htim == &htim7) && plunger_callback) {
        plunger_callback();
    } else if ((htim == &htim6) && gear_callback) {
        gear_callback();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim7) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();

        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    } else if (htim == &htim6) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM6_CLK_ENABLE();

        /* TIM6 interrupt Init */
        // TODO I do not like that we need this to go through
        // DAC. Might want to switch timers.
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
}

void initialize_linear_timer(linear_motor_interrupt_callback callback) {
    plunger_callback = callback;
    MX_TIM7_Init();
}

void initialize_gear_timer(gear_motor_interrupt_callback callback) {
    gear_callback = callback;
    MX_TIM6_Init();
}
