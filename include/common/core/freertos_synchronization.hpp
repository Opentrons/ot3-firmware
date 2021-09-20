#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace freertos_synchronization {

class FreeRTOSMutex {
  public:
    FreeRTOSMutex() { handle = xSemaphoreCreateMutexStatic(&static_data); }
    FreeRTOSMutex(const FreeRTOSMutex &) = delete;
    FreeRTOSMutex(const FreeRTOSMutex &&) = delete;
    FreeRTOSMutex &operator=(const FreeRTOSMutex &) = delete;
    FreeRTOSMutex &&operator=(const FreeRTOSMutex &&) = delete;

    ~FreeRTOSMutex() { vSemaphoreDelete(handle); }

    void acquire() { xSemaphoreTake(handle, portMAX_DELAY); }

    void release() { xSemaphoreGive(handle); }

    int get_count() { return uxSemaphoreGetCount(handle); }

  private:
    SemaphoreHandle_t handle{};
    StaticSemaphore_t static_data{};
};

}  // namespace freertos_synchronization