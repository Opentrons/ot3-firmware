/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor-control/firmware/motor_control_hardware.h"

#include "FreeRTOS.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_tim.h"
#include "stm32g4xx_hal.h"
#include "task.h"

HAL_StatusTypeDef custom_stop_pwm_it(TIM_HandleTypeDef* htim,
                                     uint32_t Channel) {
    /* Check the parameters */
    assert_param(IS_TIM_CCX_INSTANCE(htim->Instance, Channel));
    switch (Channel) {
        case TIM_CHANNEL_1: {
            /* Disable the TIM Capture/Compare 1 interrupt */
            __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
            break;
        }
        case TIM_CHANNEL_2: {
            /* Disable the TIM Capture/Compare 2 interrupt */
            __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2);
            break;
        }
        case TIM_CHANNEL_3: {
            /* Disable the TIM Capture/Compare 3 interrupt */
            __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC3);
            break;
        }
        case TIM_CHANNEL_4: {
            /* Disable the TIM Capture/Compare 4 interrupt */
            __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC4);
            break;
        }
        default:
            break;
    }
    /* Disable the Capture compare channel */
    TIM_CCxChannelCmd(htim->Instance, Channel, TIM_CCx_DISABLE);
    /* Set the TIM channel state */
    TIM_CHANNEL_STATE_SET(htim, Channel, HAL_TIM_CHANNEL_STATE_READY);
    /* Return function status */
    return HAL_OK;
}


void motor_hardware_start_encoder(void* tim_handle) {
     HAL_TIM_Encoder_Start_IT(tim_handle, TIM_CHANNEL_ALL);
}

bool motor_hardware_encoder_running(void* tim_handle) {
    if (!tim_handle) {
        return false;
    }
    return TIM_CHANNEL_STATE_GET((TIM_HandleTypeDef*)tim_handle, TIM_CHANNEL_1) == HAL_TIM_CHANNEL_STATE_BUSY;
}

void motor_hardware_stop_encoder(void* tim_handle) { HAL_TIM_Encoder_Stop_IT(tim_handle, TIM_CHANNEL_ALL); }

void motor_hardware_start_timer(void* tim_handle) { HAL_TIM_Base_Start_IT(tim_handle); }

void motor_hardware_stop_timer(void* tim_handle) { HAL_TIM_Base_Stop_IT(tim_handle); }

bool motor_hardware_timer_running(void* tim_handle) {
    if (!tim_handle) {
        return false;
    }
    return (HAL_TIM_Base_GetState(tim_handle) == HAL_TIM_STATE_BUSY);
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
    return HAL_TIM_PWM_Start_IT(htim, channel) == HAL_OK;
}

bool motor_hardware_stop_pwm(void* htim, uint32_t channel) {
    return custom_stop_pwm_it(htim, channel) == HAL_OK;
}
/*
 * read a hal timer count value
 * if the timer is NULL return 0
 */
uint16_t _get_hal_timer_count(void* htim) {
    if (htim != NULL) {
        return __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)htim);
    }
    return 0;
}

uint16_t motor_hardware_encoder_pulse_count(void* enc_htim) {
    return _get_hal_timer_count(enc_htim);
}

uint16_t motor_hardware_encoder_pulse_count_with_overflow(void* encoder_handle, int8_t *overflows) {
    if(encoder_handle == NULL) {
        return 0;
    }
    TIM_HandleTypeDef *handle = encoder_handle;
    uint32_t pulses = __HAL_TIM_GET_COUNTER(handle);
    if( __HAL_TIM_GET_UIFCPY(pulses)) {
        __HAL_TIM_CLEAR_IT(handle, TIM_FLAG_UPDATE);
        *overflows = __HAL_TIM_IS_TIM_COUNTING_DOWN(handle) ? -1 : 1;
    }
    return pulses & 0xFFFF;
}

void motor_hardware_reset_encoder_count(void* enc_htim, uint16_t reset_value) {
    __HAL_TIM_CLEAR_FLAG((TIM_HandleTypeDef*)enc_htim, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER((TIM_HandleTypeDef*)enc_htim, reset_value);
}

uint16_t motor_hardware_get_stopwatch_pulses(void* stopwatch_handle, uint8_t clear) {
    uint16_t count = _get_hal_timer_count(stopwatch_handle);
    if(stopwatch_handle != NULL && clear) {
        __HAL_TIM_SET_COUNTER((TIM_HandleTypeDef*)stopwatch_handle, 0);
    }
    return count;
}

void motor_hardware_delay(uint32_t delay) {
    const TickType_t xDelay = delay * portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
}


