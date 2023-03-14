#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "platform_specific_hal_conf.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void estop_output_gpio_init();
void utility_gpio_init();
void sync_drive_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

/* The Following variables sometimes complain since some are unused
 * some are only used in the c side or just the c++
 */

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wunused-variable"


// SYNC_MCU_OUT PA6
static GPIO_TypeDef* SYNC_MCU_OUT_PORT = GPIOA;
static uint16_t SYNC_MCU_OUT_PIN = GPIO_PIN_1;
static uint8_t SYNC_MCU_OUT_AS = GPIO_PIN_SET;
// ESTOP_MCU_OUT PA0
static GPIO_TypeDef* ESTOP_MCU_OUT_PORT = GPIOA;
static uint16_t ESTOP_MCU_OUT_PIN = GPIO_PIN_0;
static uint8_t ESTOP_MCU_OUT_AS = GPIO_PIN_RESET;
// SYNC_MCU_IN PA1
static GPIO_TypeDef* SYNC_MCU_IN_PORT = GPIOA;
static uint16_t SYNC_MCU_IN_PIN = GPIO_PIN_6;
// ESTOP_MCU_IN PA7
static GPIO_TypeDef* ESTOP_MCU_IN_PORT = GPIOA;
static uint16_t ESTOP_MCU_IN_PIN = GPIO_PIN_7;
// IO1_MCU_OUT PA4
static GPIO_TypeDef* IO1_MCU_OUT_PORT = GPIOA;
static uint16_t IO1_MCU_OUT_PIN = GPIO_PIN_4;
// IO2_MCU_OUT PA5
static GPIO_TypeDef* IO2_MCU_OUT_PORT = GPIOA;
static uint16_t IO2_MCU_OUT_PIN = GPIO_PIN_5;
// IO1_MCU_IN PC4
static GPIO_TypeDef* IO1_MCU_IN_PORT = GPIOC;
static uint16_t IO1_MCU_IN_PIN = GPIO_PIN_4;
// IO2_MCU_IN PC5
static GPIO_TypeDef* IO2_MCU_IN_PORT = GPIOC;
static uint16_t IO2_MCU_IN_PIN = GPIO_PIN_5;
// DOOR_OPEN_MCU PB0
static GPIO_TypeDef* DOOR_OPEN_MCU_PORT = GPIOB;
static uint16_t DOOR_OPEN_MCU_PIN = GPIO_PIN_0;
// AUX1_ID_MCU PB1
static GPIO_TypeDef* AUX1_ID_MCU_PORT = GPIOB;
static uint16_t AUX1_ID_MCU_PIN = GPIO_PIN_1;
// AUX2_ID_MCU PB2
static GPIO_TypeDef* AUX2_ID_MCU_PORT = GPIOB;
static uint16_t AUX2_ID_MCU_PIN = GPIO_PIN_2;
// AUX1_PRESENT_MCU PB4
static GPIO_TypeDef* AUX1_PRESENT_MCU_PORT = GPIOB;
static uint16_t AUX1_PRESENT_MCU_PIN = GPIO_PIN_4;
// AUX2_PRESENT_MCU PB6
static GPIO_TypeDef* AUX2_PRESENT_MCU_PORT = GPIOB;
static uint16_t AUX2_PRESENT_MCU_PIN = GPIO_PIN_6;
// ESTOP_DETECT_AUX2_MCU PB12
static GPIO_TypeDef* ESTOP_DETECT_AUX2_MCU_PORT = GPIOB;
static uint16_t ESTOP_DETECT_AUX2_MCU_PIN = GPIO_PIN_12;
// ESTOP_DETECT_AUX1_MCU PB13
static GPIO_TypeDef* ESTOP_DETECT_AUX1_MCU_PORT = GPIOB;
static uint16_t ESTOP_DETECT_AUX1_MCU_PIN = GPIO_PIN_13;

#pragma GCC diagnostic pop