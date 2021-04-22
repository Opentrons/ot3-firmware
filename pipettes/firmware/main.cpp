

#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32f3xx.h"


auto main() -> int {
    HardwareInit();
    vTaskStartScheduler();
    return 0;
}