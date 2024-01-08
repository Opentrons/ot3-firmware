#include "common/firmware/clocking.h"

#include "common/firmware/errors.h"
#include "stm32g4xx_hal_conf.h"

void RCC_Peripheral_Clock_Select() {
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection =
        RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_FDCAN | RCC_PERIPHCLK_I2C2 |
        RCC_PERIPHCLK_I2C3;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}
