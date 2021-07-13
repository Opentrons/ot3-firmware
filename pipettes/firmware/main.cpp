#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "common/firmware/spi.h"
#include "common/firmware/uart.h"
#include "firmware/common/uart_comms.hpp"
#include "pipettes/core/communication.hpp"
#include "pipettes/firmware/can_task.hpp"
#include "pipettes/firmware/uart_task.hpp"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
    uart_task::start();
    can_task::start();

    vTaskStartScheduler();

    return 0;
}
