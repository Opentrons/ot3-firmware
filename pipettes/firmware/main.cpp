#include "FreeRTOS.h"
#include "STM32G491RETx/system_stm32g4xx.h"
#include "task.h"

auto main() -> int {
    HardwareInit();
    vTaskStartScheduler();

    return 0;
}
