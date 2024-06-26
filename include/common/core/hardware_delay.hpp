#pragma once
// Not my favorite way to check this, but if we don't have access
// to vTaskDelay during host compilation so just dummy the function

#ifdef ENABLE_CROSS_ONLY_HEADERS
#include "FreeRTOS.h"
#endif

template <typename T>
requires std::is_integral_v<T>
static void vtask_hardware_delay(T ticks) {
#ifdef ENABLE_CROSS_ONLY_HEADERS
    vTaskDelay(ticks);
#else
    std::ignore = ticks;
#endif
}
