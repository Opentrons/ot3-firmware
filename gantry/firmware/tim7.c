#include "common/firmware/errors.h"
#include "common/firmware/timer_interrupt.h"
#include "stm32g4xx_hal.h"

TIM_HandleTypeDef htim7;

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

    /*Configure GPIO pin : LD2_Pin */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 500;
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
    if (htim == &htim7) {
        step_motor();
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

void timer_init() {
    MX_GPIO_Init();
    MX_TIM7_Init();
}

void timer_interrupt_start() { HAL_TIM_Base_Start_IT(&htim7); }

void timer_interrupt_stop() { HAL_TIM_Base_Stop_IT(&htim7); }

void turn_on_step_pin() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); }

void turn_off_step_pin() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

void turn_on_direction_pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}

void turn_off_direction_pin() {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
}
