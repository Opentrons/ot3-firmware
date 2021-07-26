/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */
#include "stm32g4xx_hal_conf.h"

void Set_Direction() { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); }

void Set_Step() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); }

void Reset_Step() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); }

void Reset_Direction() { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); }

void Set_Enable_Pin() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); }

void Reset_Enable_Pin() { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); }
