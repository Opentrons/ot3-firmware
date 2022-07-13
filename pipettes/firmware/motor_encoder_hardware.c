#include "motor_encoder_hardware.h"
#include "common/firmware/errors.h"
#include "stm32g4xx_hal.h"
#include "pipettes/core/pipette_type.h"

TIM_HandleTypeDef htim2;

void Encoder_GPIO_Init(PipetteType pipette_type){
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    /* Encoder P Axis GPIO Configuration
    PA0     ------> CHANNEL B ----> GPIO_PIN_0
    PA1     ------> CHANNEL A ----> GPIO_PIN_1
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /*Encoder P Axis Index Pin Configuration
    PA5    ------> CHANNEL I -----> SINGLE_CHANNEL/EIGHT_CHANNEL
    PA7    ------> CHANNEL I -----> NINETY_SIX_CHANNEL/THREE_EIGHTY_FOUR_CHANNEL
    * The Encoder Index Pin is routed to different GPIO pins
    * depending on the pipette type.
    */
    if (pipette_type == NINETY_SIX_CHANNEL || pipette_type == THREE_EIGHTY_FOUR_CHANNEL){
            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        }
    else{
            GPIO_InitStruct.Pin = GPIO_PIN_5;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }

}
/**
  * This currently only works for the Lseries
  * TODO/CF Change this when single pipette boards switch to STMG491 MCU
*/
void TIM2_EncoderP_Init(void){
    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
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
    /* Start Encoder */
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

void initialize_enc(PipetteType pipette_type) {Encoder_GPIO_Init(pipette_type); TIM2_EncoderP_Init();}
