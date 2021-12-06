#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/adc.h"
#pragma GCC diagnostic pop

void task_entry_() {
    adc_setup();
    while (true) {
        adc_read_voltages();
    }
}

auto static task = freertos_task::FreeRTOSTask<512, 5>("adc task", task_entry_);
