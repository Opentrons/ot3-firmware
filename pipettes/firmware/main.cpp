#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal.h"
#include "task.h"
// clang-format on

#include "common/firmware/clocking.h"
#include "common/firmware/can_task.hpp"


auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    can_task::start();

    vTaskStartScheduler();
}
