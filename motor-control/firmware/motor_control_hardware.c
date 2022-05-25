/*
 * A C file to act as a middle layer to HAL related functions for GPIO.
 */

#include "motor-control/firmware/motor_control_hardware.h"

#include "FreeRTOS.h"
#include "platform_specific_hal_conf.h"
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
    return HAL_TIM_PWM_Start_IT(htim, channel) == HAL_OK;
}

bool motor_hardware_stop_pwm(void* htim, uint32_t channel) {
    return custom_stop_pwm_it(htim, channel) == HAL_OK;
}
/*
 * On the current prototype there are no encoders on XY axes, to handle that
 * this NULL condition was made to return a zero value for pulse counts.
 *
 * Note: Eventually we can remove these if statements when we get encoders on XY
 * Axes
 */
uint32_t motor_hardware_encoder_pulse_count(void* enc_htim) {
    uint32_t pulses;
    if (enc_htim != NULL) {
        pulses = __HAL_TIM_GET_COUNTER((TIM_HandleTypeDef*)enc_htim);
    } else {
        pulses = 0;
    }
    return pulses;
}

void motor_hardware_reset_encoder_count(void* enc_htim) {
    if (enc_htim != NULL) {
        __HAL_TIM_SET_COUNTER((TIM_HandleTypeDef*)enc_htim, 0);
    }
}

/*
* These functions will be used to determine the counter overflow on the encoder
* We need a way to clear the status register and getting the status register flag bit
*/
void motor_hardware_clear_status_register(void *enc_htim){
    if (enc_htim != NULL){
        /* Clear interrupt flag bit */
        __HAL_TIM_CLEAR_IT((TIM_HandleTypeDef*)enc_htim, TIM_IT_UPDATE);
        //  /* The update event of the enable timer is interrupted */
        // __HAL_TIM_ENABLE_IT((TIM_HandleTypeDef*)enc_htim, TIM_IT_UPDATE);
        // /* Set update event request source as: counter overflow */
        // __HAL_TIM_URS_ENABLE((TIM_HandleTypeDef*)enc_htim);
    }
}

bool motor_hardware_encoder_get_status_register(void *enc_htim){
    bool status_register;
    if (enc_htim != NULL){
        status_register = __HAL_TIM_GET_FLAG((TIM_HandleTypeDef*)enc_htim, TIM_FLAG_UPDATE);
    }
    else{
        status_register = 0;
    }
    return status_register;
}

bool motor_hardware_encoder_get_direction(void *enc_htim){
    bool direction;
    if (enc_htim != NULL){
        direction = __HAL_TIM_IS_TIM_COUNTING_DOWN((TIM_HandleTypeDef*)enc_htim);
    }
    else{
        direction = 0;
    }
    return direction;
}