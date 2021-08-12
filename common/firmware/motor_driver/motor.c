/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor_gpio_conf.h"
#include "stm32g4xx_hal_conf.h"

void Set_Direction() {
    HAL_GPIO_WritePin(DIRECTION_GPIO_PORT, DIRECTION_PIN, GPIO_PIN_SET);
}

void Set_Step() { HAL_GPIO_WritePin(STEP_GPIO_PORT, STEP_PIN, GPIO_PIN_SET); }

void Reset_Step() {
    HAL_GPIO_WritePin(STEP_GPIO_PORT, STEP_PIN, GPIO_PIN_RESET);
}

void Reset_Direction() {
    HAL_GPIO_WritePin(DIRECTION_GPIO_PORT, DIRECTION_PIN, GPIO_PIN_RESET);
}

void Set_Enable_Pin() {
    HAL_GPIO_WritePin(ENABLE_GPIO_PORT, ENABLE_PIN, GPIO_PIN_SET);
}

void Reset_Enable_Pin() {
    HAL_GPIO_WritePin(ENABLE_GPIO_PORT, ENABLE_PIN, GPIO_PIN_RESET);
}
