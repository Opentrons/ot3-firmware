#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "task.h"
// clang-format on

#include "common/firmware/can_task.hpp"
#include "common/firmware/clocking.h"
#include "common/firmware/spi.h"
#include "common/firmware/uart.h"
#include "common/firmware/uart_comms.hpp"
#include "common/firmware/uart_task.hpp"
#include "pipettes/core/communication.hpp"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    uart_task::start();
    can_task::start();

    vTaskStartScheduler();

    return 0;
}
