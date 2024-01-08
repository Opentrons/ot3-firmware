#include "bootloader/firmware/system.h"
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"

// This file should only be compiled by the head project. It provides
// a startup function that sets up the mount ID lines.

void system_specific_startup() {
    // Z/left: PC5
    // A/right: PB2
    // G (unused): PB1
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // z/left is pulldown to conform to old EVT external pulls
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    // gripper is pulldown because it's not used by gripper and might
    // as well be grounded
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    // a/right is pullup to differentiate it from z/left
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
