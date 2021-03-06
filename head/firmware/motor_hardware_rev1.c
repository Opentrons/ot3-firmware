#include "motor_hardware.h"
#include "stm32g4xx_hal.h"

void initialize_rev_specific_pins() {
    // on rev1, we have electromagnetic brakes
    // for both motors and a correct sense for
    // the right motor enable pin
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_5 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,
                      GPIO_PIN_RESET);
    // TODO: Handle brakes better, this just disengages
    // them at boot
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5 | GPIO_PIN_11, GPIO_PIN_SET);
}
