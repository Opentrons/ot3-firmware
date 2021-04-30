

#include "FreeRTOS.h"
#include "system_stm32f3xx.h"
#include "task.h"

auto main() -> int {
    HardwareInit();
    vTaskStartScheduler();
    return 0;
}