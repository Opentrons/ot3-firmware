#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "common/core/freertos_timer.hpp"
#include "common/firmware/clocking.h"

static void callback() {}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();

    // Dummy timer code, should be removed once SW timers are implemented
    auto timer = freertos_timer::FreeRTOSTimer("test timer", callback);
    timer.start();

    vTaskStartScheduler();
}
