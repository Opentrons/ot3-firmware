#ifndef MOTOR_CONTROL_HARDWARE_H_
#define MOTOR_CONTROL_HARDWARE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
void motor_hardware_start_timer(void* tim_handle);
void motor_hardware_stop_timer(void* tim_handle);
bool motor_hardware_start_dac(void* dac_handle, uint32_t channel);
bool motor_hardware_stop_dac(void* dac_handle, uint32_t channel);
bool motor_hardware_set_dac_value(void* dac_handle, uint32_t channel,
                                  uint32_t data_algn, uint32_t val);
bool motor_hardware_start_pwm(void* tim_handle, uint32_t channel);
bool motor_hardware_stop_pwm(void* tim_handle, uint32_t channel);
int32_t motor_hardware_encoder_pulse_count(void* encoder_handle);
void motor_hardware_reset_encoder_count(void* encoder_handle);
void motor_hardware_clear_status_register(void* encoder_handle);
bool motor_hardware_encoder_get_status_register(void* encoder_handle);
bool motor_hardware_encoder_get_direction(void* encoder_handle);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // MOTOR_CONTROL_HARDWARE_H_
