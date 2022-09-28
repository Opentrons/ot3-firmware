#include "motor_encoder_hardware.h"

#include "common/firmware/errors.h"
#include "stm32g4xx_hal.h"

#define GRIPPER_ENCODER_SPEED_TIMER_FREQ (1000000UL)

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

uint32_t round_closest(uint32_t dividend, uint32_t divisor) {
    return (dividend + (divisor / 2)) / divisor;
}

uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq) {
    return timer_clk_freq >= counter_clk_freq
               ? round_closest(timer_clk_freq, counter_clk_freq) - 1U
               : 0U;
}

void Encoder_GPIO_Init(void) {
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    /* Encoder G Axis GPIO Configuration
    PA0     ------> CHANNEL A
    PA1     ------> CHANNEL B
    PA5    ------> CHANNEL I
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM2_EncoderG_Init(void) {
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
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
    sConfig.IC2Polarity = TIM_ICPOLARITY_FALLING;
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
void TIM4_EncoderGSpeed_Init(void) {
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    htim4.Instance = TIM4;
    htim4.Init.Prescaler =
        calc_prescaler(SystemCoreClock, GRIPPER_ENCODER_SPEED_TIMER_FREQ);
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 3000;  // timer overflows after 3 ms
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
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
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
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

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
        /* TIM2 interrupt Init */
        HAL_NVIC_SetPriority(TIM2_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void initialize_enc() {
    Encoder_GPIO_Init();
    TIM2_EncoderG_Init();
    TIM4_EncoderGSpeed_Init();
}
