#pragma once

#include "FreeRTOS.h"
#include "task.h"

namespace ot_utils {
namespace freertos_sleep {

static void sleep(uint32_t time_ms) { if (time_ms > 0) vTaskDelay(pdMS_TO_TICKS(time_ms)); }

}  // namespace freertos_sleep
}  // namespace ot_utils
