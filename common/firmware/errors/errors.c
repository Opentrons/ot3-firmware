#include "common/firmware/errors.h"

#include "stm32g4xx_hal_conf.h"

void Error_Handler(void) {
    // TODO: add implementation to report the HAL error return state
    __disable_irq();
    while (1) {
    }
}