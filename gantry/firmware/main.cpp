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
#include "common/firmware/utility_config.h"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_init();

    vTaskStartScheduler();
}
