#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "stm32g4xx_hal_gpio.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void utility_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

/* The Following variables sometimes complain since some are unused
 * some are only used in the c side or just the c++
 */

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wunused-variable"


/* ---- Generic Pins ---- */

// LED_DRIVE PA1
#define LED_DRIVE_PORT GPIOA
#define LED_DRIVE_PIN GPIO_PIN_1
#define LED_DRIVE_AS GPIO_PIN_SET
// DOOR_OPEN_MCU PC7
#define DOOR_OPEN_MCU_PORT GPIOC
#define DOOR_OPEN_MCU_PIN GPIO_PIN_7
#define DOOR_OPEN_MCU_AS GPIO_PIN_SET
// REED_SW_MCU PC11
#define REED_SW_MCU_PORT GPIOC
#define REED_SW_MCU_PIN GPIO_PIN_11
#define REED_SW_MCU_AS GPIO_PIN_SET
// AUX_ID_MCU PC12
#define AUX_ID_MCU_PORT GPIOC
#define AUX_ID_MCU_PIN GPIO_PIN_12
// CAN_RXD


/* ---- Hepa Pins ---- */

// HEPA_NO_MCU PB10
#define HEPA_NO_MCU_PORT GPIOB
#define HEPA_NO_MCU_PIN GPIO_PIN_10
// HEPA_FG_MCU PA5
#define HEPA_FG_MCU_PORT GPIOA
#define HEPA_FG_MCU_PIN GPIO_PIN_5
// HEPA_PWM PA6
#define HEPA_PWM_PORT GPIOA
#define HEPA_PWM_PIN GPIO_PIN_6
// HEPA_ON/OFF PA7
#define HEPA_ON_OFF_PORT GPIOA
#define HEPA_ON_OFF_PIN GPIO_PIN_7
// HEPA_R_CTRL PA9
#define HEPA_R_CTRL_PORT GPIOA
#define HEPA_R_CTRL_PIN GPIO_PIN_9
// HEPA_W_CTRL PA10
#define HEPA_W_CTRL_PORT GPIOA
#define HEPA_W_CTRL_PIN GPIO_PIN_10
// HEPA_G_CTRL PC6
#define HEPA_G_CTRL_PORT GPIOC
#define HEPA_G_CTRL_PIN GPIO_PIN_6
// HEPA_B_CTRL PB4
#define HEPA_B_CTRL_PORT GPIOB
#define HEPA_B_CTRL_PIN GPIO_PIN_4


/* ---- UV Pins ---- */

// UV_SNS_MCU PA0
#define UV_SNS_MCU_PORT GPIOA
#define UV_SNS_MCU_PIN GPIO_PIN_0
// UV_ON_OFF_MCU PA4
#define UV_ON_OFF_MCU_PORT GPIOA
#define UV_ON_OFF_MCU_PIN GPIO_PIN_4
// UV_NO_MCU PC2
#define UV_NO_MCU_PORT GPIOC
#define UV_NO_MCU_PIN GPIO_PIN_2
// UV_ADC PA3
#define UV_ADC_PORT GPIOC
#define UV_ADC_PIN GPIO_PIN_3
// UV_B_CTRL PC5
#define UV_B_CTRL_PORT GPIOC
#define UV_B_CTRL_PIN GPIO_PIN_5
// UV_G_CTRL PB0
#define UV_G_CTRL_PORT GPIOB
#define UV_G_CTRL_PIN GPIO_PIN_0
// UV_R_CTRL PB1
#define UV_R_CTRL_PORT GPIOB
#define UV_R_CTRL_PIN GPIO_PIN_1
// UV_W_CTRL PB2
#define UV_W_CTRL_PORT GPIOB
#define UV_W_CTRL_PIN GPIO_PIN_2