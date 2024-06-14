#pragma once
// Not my favorite way to check this, but if we don't have access
// to vTaskDelay during host compilation so just dummy the function
template <typename T>
requires std::is_integral_v<T>
static void vtask_hardware_delay(T ticks) {
#ifndef INC_TASK_H
    std::ignore = ticks;
#else
    vTaskDelay(ticks);
#endif
}
