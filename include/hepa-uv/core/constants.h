#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// the number of selectable points in the PWM
#define PWM_WIDTH (256UL)

typedef enum LED_TYPE {
	RED_LED,
	GREEN_LED,
	BLUE_LED,
	WHITE_LED
} LED_TYPE;

typedef enum PUSH_BUTTON_TYPE {
	HEPA_BUTTON,
	UV_BUTTON
} PUSH_BUTTON_TYPE;

typedef struct LEDState {
	LED_TYPE led;
	uint32_t duty_cycle;
} LEDState;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
