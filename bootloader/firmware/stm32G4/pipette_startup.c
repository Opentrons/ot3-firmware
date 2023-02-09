#include "bootloader/firmware/system.h"
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "stm32g4xx_hal_gpio.h"

// This file should only be compiled by the pipette project. It provides
// a startup function that sets up the sync output line.

void system_specific_startup() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
}
