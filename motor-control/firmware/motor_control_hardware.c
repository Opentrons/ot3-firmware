/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor_control_hardware.h"

#include "FreeRTOS.h"
#include "platform_specific_hal_conf.h"
#include "task.h"

void motor_hardware_set_pin(void* port, uint16_t pin, uint8_t active_setting) {
    if (!port) {
        return;
    }
    HAL_GPIO_WritePin(port, pin, active_setting);
}

static uint8_t invert_gpio_value(uint8_t setting) {
    switch (setting) {
        case GPIO_PIN_SET:
            return GPIO_PIN_RESET;
        case GPIO_PIN_RESET:
            return GPIO_PIN_SET;
        default:
            return GPIO_PIN_SET;
    }
}

void motor_hardware_reset_pin(void* port, uint16_t pin,
                              uint8_t active_setting) {
    if (!port) {
        return;
    }
    HAL_GPIO_WritePin(port, pin, invert_gpio_value(active_setting));
}

void motor_hardware_start_timer(void* htim) { HAL_TIM_Base_Start_IT(htim); }

void motor_hardware_stop_timer(void* htim) { HAL_TIM_Base_Stop_IT(htim); }

bool motor_hardware_get_pin_value(void* port, uint16_t pin,
                                  uint8_t active_setting) {
    return HAL_GPIO_ReadPin(port, pin) == active_setting;
}

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

uint32_t motor_hardware_encoder_pulse_count(void *enc_htim){
    uint32_t pulses = __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)enc_htim);
    return pulses;
}
