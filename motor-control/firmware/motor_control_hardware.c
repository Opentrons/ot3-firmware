/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor_control_hardware.h"

#include "FreeRTOS.h"
#include "platform_specific_hal_conf.h"
#include "task.h"

void motor_hardware_start_timer(void* htim) { HAL_TIM_Base_Start_IT(htim); }

void motor_hardware_stop_timer(void* htim) { HAL_TIM_Base_Stop_IT(htim); }

bool motor_hardware_start_dac(void* hdac, uint32_t channel) {
    return HAL_DAC_Start(hdac, channel) == HAL_OK;
}

bool motor_hardware_stop_dac(void* hdac, uint32_t channel) {
    return HAL_DAC_Stop(hdac, channel) == HAL_OK;
}

bool motor_hardware_set_dac_value(void* hdac, uint32_t channel,
                                  uint32_t data_algn, uint32_t val) {
    return HAL_DAC_SetValue(hdac, channel, data_algn, val) == HAL_OK;
}

bool motor_hardware_start_pwm(void* htim, uint32_t channel) {
    return HAL_TIM_PWM_Start(htim, channel) == HAL_OK;
}

bool motor_hardware_stop_pwm(void* htim, uint32_t channel) {
    return HAL_TIM_PWM_Stop(htim, channel) == HAL_OK;
}
