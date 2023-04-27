#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "gripper/firmware/utility_gpio.h"
#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_it.h"

// The frequency of one full PWM cycle
#define GRIPPER_JAW_PWM_FREQ_HZ (32000UL)
// the number of selectable points in the PWM
#define GRIPPER_JAW_PWM_WIDTH (100UL)
// the frequency at which the timer should count so that it
// does a full PWM cycle in the time specified by GRIPPER_JAW_PWM_FREQ_HZ
#define GRIPPER_JAW_TIMER_FREQ (GRIPPER_JAW_PWM_FREQ_HZ * GRIPPER_JAW_PWM_WIDTH)

#define GRIPPER_ENCODER_SPEED_TIMER_FREQ (1000000UL)
// these values create the longest possible clock with at 16bit counter
#define GRIPPER_ENCODER_FORCE_STOPWATCH_FREQ (2600UL)
#define GRIPPER_ENCODER_FORCE_STOPWATCH_PERIOD (65000UL)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Z motor specific
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;

typedef void (*motor_interrupt_callback)();
typedef void (*z_encoder_overflow_callback)(int32_t);


HAL_StatusTypeDef initialize_spi();
void initialize_hardware_z();

void set_z_motor_timer_callback(
        motor_interrupt_callback callback,
        z_encoder_overflow_callback enc_callback);


// G motor specific
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim15;

typedef void (*brushed_motor_interrupt_callback)();
typedef void (*encoder_overflow_callback)(int32_t);
typedef void (*encoder_idle_state_callback)(bool);
typedef void (*stopwatch_overflow_callback)(uint16_t seconds);

void initialize_hardware_g();

void update_pwm(uint32_t duty_cycle);

void set_brushed_motor_timer_callback(
    brushed_motor_interrupt_callback callback,
    encoder_overflow_callback g_enc_f_callback,
    encoder_idle_state_callback g_enc_idle_callback,
    stopwatch_overflow_callback g_stopwatch_overflow_callback);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
