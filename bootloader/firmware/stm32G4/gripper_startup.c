#include "bootloader/firmware/system.h"
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "stm32g4xx_hal_gpio.h"

// This file should only be compiled by the gripper project.
// It provides a startup function that properly configures the mount detect
// pin.

void system_specific_startup() {
    // We need to set our tool id pin to output high so that the mount knows
    // we're here. Unlike the pipette, we're not detecting what mount we're on
    // since there's just one so we don't need to start out as input.
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}
