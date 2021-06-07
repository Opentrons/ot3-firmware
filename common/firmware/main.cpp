#include <system/FreeRTOS.h>
#include <system/system_stm32g4xx.h>
#include <system/task.h>

auto main() -> int {
    HardwareInit();
    vTaskStartScheduler();

    return 0;
}
