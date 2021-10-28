/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "common/firmware/motor.h"

#include "FreeRTOS.h"
#include "task.h"

#include "platform_specific_hal_conf.h"

void set_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_SET);
}

void reset_pin(struct PinConfig config) {
    HAL_GPIO_WritePin(config.port, config.pin, GPIO_PIN_RESET);
}

void delay(const int seconds) { vTaskDelay(seconds); }
