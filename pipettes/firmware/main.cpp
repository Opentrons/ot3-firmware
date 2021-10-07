#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "task.h"
// clang-format on

#include "common/firmware/can_task.hpp"
#include "common/firmware/clocking.h"
#include "pipettes/core/communication.hpp"
#include "common/firmware/uart_task.hpp"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    can_task::start();
    uart_task::start();

    vTaskStartScheduler();
}
