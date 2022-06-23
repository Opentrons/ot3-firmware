#include "motor_hardware.h"
#include "stm32g4xx_hal.h"


void initialize_rev_specific_pins() {
    // on the proto board, b0 goes to the tmc2130
    // clock and must be tied low; and the right motor enable
    // pin has a swapped sense and is active low instead of high
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_11, GPIO_PIN_RESET);
}
