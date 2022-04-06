/*
 * Configuration for the FreeRTOS idle task, which is necessary when we told it
 * we're using static allocation. Provides the same configuration as the other
 * stacks, but in callback form (vApplicationGetIdleTaskMemory is called by the
 * RTOS internals)
 *
 * This file is only present to support the FreeRTOS POSIX Port.
 */

#include <array>

#include "FreeRTOS.h"
#include "task.h"

StaticTask_t
    idle_task_tcb;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::array<StackType_t, configMINIMAL_STACK_SIZE>
    idle_task_stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

StaticTask_t
    timer_task_tcb;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::array<StackType_t, configMINIMAL_STACK_SIZE * 2>
    timer_task_stack;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

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
    *ppxTimerTaskTCBBuffer = &timer_task_tcb;
    *ppxTimerTaskStackBuffer = timer_task_stack.data();
    *pulTimerTaskStackSize = timer_task_stack.size();
}
