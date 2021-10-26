/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "common/firmware/motor.h"

#include "FreeRTOS.h"
#include "task.h"

#if __has_include("stm32l5xx_hal_conf.h")
#include "stm32l5xx_hal_conf.h"
#else
#include "stm32g4xx_hal_conf.h"
#endif

void set_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_SET);
}

void reset_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_RESET);
}

void delay(const int seconds) { vTaskDelay(seconds); }
