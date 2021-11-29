#ifndef MOTOR_CONTROL_HARDWARE_H_
#define MOTOR_CONTROL_HARDWARE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void motor_hardware_set_pin(void* port, uint16_t pin, uint8_t active_setting);
void motor_hardware_reset_pin(void* port, uint16_t pin, uint8_t active_setting);
void motor_hardware_start_timer(void* tim_handle);
void motor_hardware_stop_timer(void* tim_handle);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus


#endif // MOTOR_CONTROL_HARDWARE_H_
