/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor_control_hardware.h"

#include "FreeRTOS.h"
#include "task.h"

#include "platform_specific_hal_conf.h"

void motor_hardware_set_pin(void* port, uint16_t pin, uint8_t active_setting) {
    HAL_GPIO_WritePin(port, pin, active_setting);
}

void motor_hardware_reset_pin(void* port, uint16_t pin, uint8_t active_setting) {
    HAL_GPIO_WritePin(
        port, pin,
        (active_setting == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_RESET);
}

void motor_hardware_start_timer(void* htim) { HAL_TIM_Base_Start_IT(htim); }

void motor_hardware_stop_timer(void* htim) { HAL_TIM_Base_Stop_IT(htim); }
