#pragma once
/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "common/firmware/motor.h"

#include "FreeRTOS.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"

void set_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_SET);
}

void reset_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_RESET);
}

void delay(const int seconds) { vTaskDelay(seconds); }
