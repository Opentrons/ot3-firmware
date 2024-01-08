/**
 * @file constants.h
 * @brief Constants that are specific to the hepa-uv and shared across
 * host & cross builds. This must be a plain C file to accomadate the firmware
 * targets.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// the number of selectable points in the PWM
#define LED_PWM_WIDTH (256UL)

typedef enum LED_TYPE {
	DECK_LED,
	RED_UI_LED,
	GREEN_UI_LED,
	BLUE_UI_LED,
	WHITE_UI_LED
}LED_DEVICE;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus