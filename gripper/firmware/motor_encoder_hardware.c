#include "motor_encoder_hardware.h"
#include "common/firmware/errors.h"
#include "stm32g4xx_hal.h"

TIM_HandleTypeDef htim2;

void Encoder_GPIO_Init(void){
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    /* Encoder G Axis GPIO Configuration
    PA0     ------> CHANNEL B
    PA1     ------> CHANNEL A
    PA5    ------> CHANNEL I
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM2_EncoderG_Init(void){
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = UINT16_MAX;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 0;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 0;
    if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
    {
    Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
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

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) {
    if (htim == &htim2) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
        /* TIM2 interrupt Init */
        HAL_NVIC_SetPriority(TIM2_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void initialize_enc() 
{
    Encoder_GPIO_Init(); 
    TIM2_EncoderG_Init();
}
