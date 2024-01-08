#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void utility_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

// DOOR_OPEN_MCU PC7
#define DOOR_OPEN_MCU_PORT GPIOC
#define DOOR_OPEN_MCU_PIN GPIO_PIN_7
#define DOOR_OPEN_MCU_AS GPIO_PIN_SET

// REED_SW_MCU PC11
#define REED_SW_MCU_PORT GPIOC
#define REED_SW_MCU_PIN GPIO_PIN_11
#define REED_SW_MCU_AS GPIO_PIN_SET

// LED_DRIVE PA1
#define LED_DRIVE_PORT GPIOA
#define LED_DRIVE_PIN GPIO_PIN_1
#define LED_DRIVE_AS GPIO_PIN_SET

// AUX_ID_MCU PC12
#define AUX_ID_MCU_PORT GPIOC
#define AUX_ID_MCU_PIN GPIO_PIN_12
#define AUX_ID_MCU_AS GPIO_PIN_SET