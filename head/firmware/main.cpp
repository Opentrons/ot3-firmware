#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#pragma GCC diagnostic pop

#include "common/firmware/can_task.hpp"
#include "common/firmware/clocking.h"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    vTaskStartScheduler();
}
