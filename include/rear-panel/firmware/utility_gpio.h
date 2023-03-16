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
#define SYNC_MCU_OUT_PORT GPIOA
#define SYNC_MCU_OUT_PIN GPIO_PIN_1
#define SYNC_MCU_OUT_AS GPIO_PIN_SET
// ESTOP_MCU_OUT PA0
#define ESTOP_MCU_OUT_PORT GPIOA
#define ESTOP_MCU_OUT_PIN GPIO_PIN_0
#define ESTOP_MCU_OUT_AS GPIO_PIN_RESET
// SYNC_MCU_IN PA1
#define SYNC_MCU_IN_PORT GPIOA
#define SYNC_MCU_IN_PIN GPIO_PIN_6
// ESTOP_MCU_IN PA7
#define ESTOP_MCU_IN_PORT GPIOA
#define ESTOP_MCU_IN_PIN GPIO_PIN_7
// IO1_MCU_OUT PA4
#define IO1_MCU_OUT_PORT GPIOA
#define IO1_MCU_OUT_PIN GPIO_PIN_4
// IO2_MCU_OUT PA5
#define IO2_MCU_OUT_PORT GPIOA
#define IO2_MCU_OUT_PIN GPIO_PIN_5
// IO1_MCU_IN PC4
#define IO1_MCU_IN_PORT GPIOC
#define IO1_MCU_IN_PIN GPIO_PIN_4
// IO2_MCU_IN PC5
#define IO2_MCU_IN_PORT GPIOC
#define IO2_MCU_IN_PIN GPIO_PIN_5
// DOOR_OPEN_MCU PB0
#define DOOR_OPEN_MCU_PORT GPIOB
#define DOOR_OPEN_MCU_PIN GPIO_PIN_0
// AUX1_ID_MCU PB1
#define AUX1_ID_MCU_PORT GPIOB
#define AUX1_ID_MCU_PIN GPIO_PIN_1
// AUX2_ID_MCU PB2
#define AUX2_ID_MCU_PORT GPIOB
#define AUX2_ID_MCU_PIN GPIO_PIN_2
// AUX1_PRESENT_MCU PB4
#define AUX1_PRESENT_MCU_PORT GPIOB
#define AUX1_PRESENT_MCU_PIN GPIO_PIN_4
// AUX2_PRESENT_MCU PB6
#define AUX2_PRESENT_MCU_PORT GPIOB
#define AUX2_PRESENT_MCU_PIN GPIO_PIN_6
// ESTOP_DETECT_AUX2_MCU PB12
#define ESTOP_DETECT_AUX2_MCU_PORT GPIOB
#define ESTOP_DETECT_AUX2_MCU_PIN GPIO_PIN_12
// ESTOP_DETECT_AUX1_MCU PB13
#define ESTOP_DETECT_AUX1_MCU_PORT GPIOB
#define ESTOP_DETECT_AUX1_MCU_PIN GPIO_PIN_13

#pragma GCC diagnostic pop