/*
 * Configuration for the FreeRTOS idle task, which is necessary when we told it
 * we're using static allocation. Provides the same configuration as the other
 * stacks, but in callback form (vApplicationGetIdleTaskMemory is called by the
 * RTOS internals)
 */

#include <array>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

StaticTask_t
    idle_task_tcb;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

StaticTask_t
    idle_timer_tcb;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::array<StackType_t, 512>
    idle_task_stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::array<StackType_t, 512>
    idle_timer_stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// This is a callback defined in a C file so it has to be linked as such
extern "C" void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize) {
    // Same configuration as in the other tasks, but a smaller stack
    *ppxIdleTaskTCBBuffer = &idle_task_tcb;
    *ppxIdleTaskStackBuffer = idle_task_stack.data();
    *pulIdleTaskStackSize = idle_task_stack.size();
}

extern "C" void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &idle_timer_tcb;
    *ppxTimerTaskStackBuffer = idle_timer_stack.data();
    *pulTimerTaskStackSize = idle_timer_stack.size();
}

// We are matching the definition of this function in FreeRTOS, and thus
// keep the function signature the same
extern "C" void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    // NOLINTNEXTLINE(readability-non-const-parameter)
    signed char *pcTaskName) {
    static_cast<void>(xTask);
    static_cast<void>(pcTaskName);
    // Lock the processor forever
    configASSERT(0);
}
