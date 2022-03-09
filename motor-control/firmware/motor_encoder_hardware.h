
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
void motor_encoder_set_pin(void* port, uint16_t pin, uint8_t active_setting);
void motor_encoder_reset_pin(void* port, uint16_t pin, uint8_t active_setting);
void motor_encoder_start_timer(void* tim_handle);
void motor_encoder_stop_timer(void* tim_handle);
uint32_t motor_encoder_get_count(void *tim_handle, uint32_t Channel);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus


#endif // MOTOR_CONTROL_HARDWARE_H_
