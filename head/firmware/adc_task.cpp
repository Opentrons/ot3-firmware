#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/adc.h"
#pragma GCC diagnostic pop

// call adc config methos
// call adc read method from free RTOS task

void task_entry_() {
    adc_setup();
    while (1) {
        adc_read_voltages();
    }
    // call function that reads voltage here!
}

auto static task = freertos_task::FreeRTOSTask<512, 5>("adc task", task_entry_);