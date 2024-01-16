#ifndef MOTOR_CONTROL_HARDWARE_H_
#define MOTOR_CONTROL_HARDWARE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
void motor_hardware_start_timer(void* tim_handle);
void motor_hardware_stop_timer(void* tim_handle);
void motor_hardware_start_encoder(void* tim_handle);
void motor_hardware_stop_encoder(void* tim_handle);
bool motor_hardware_encoder_running(void* tim_handle);
bool motor_hardware_timer_running(void* tim_handle);
bool motor_hardware_start_dac(void* dac_handle, uint32_t channel);
bool motor_hardware_stop_dac(void* dac_handle, uint32_t channel);
bool motor_hardware_set_dac_value(void* dac_handle, uint32_t channel,
                                  uint32_t data_algn, uint32_t val);
bool motor_hardware_start_pwm(void* tim_handle, uint32_t channel);
bool motor_hardware_stop_pwm(void* tim_handle, uint32_t channel);
uint16_t motor_hardware_encoder_pulse_count(void* encoder_handle);
uint16_t motor_hardware_encoder_pulse_count_with_overflow(void* encoder_handle, int8_t *overflows);
void motor_hardware_reset_encoder_count(void* encoder_handle, uint16_t reset_value);
uint16_t motor_hardware_get_stopwatch_pulses(void* stopwatch_handle, uint8_t clear);
void motor_hardware_delay(uint32_t delay);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // MOTOR_CONTROL_HARDWARE_H_
