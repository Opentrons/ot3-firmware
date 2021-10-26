#include "common/firmware/errors.h"

#if __has_include("stm32l5xx_hal_conf.h")
#include "stm32l5xx_hal_conf.h"
#else
#include "stm32g4xx_hal_conf.h"
#endif

void Error_Handler(void) {
    // TODO: add implementation to report the HAL error return state
    __disable_irq();
    while (1) {
    }
}