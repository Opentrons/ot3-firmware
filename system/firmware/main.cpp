#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"

auto main() -> int {
    HardwareInit();
    vTaskStartScheduler();
    return 0;
}
