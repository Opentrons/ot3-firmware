#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace freertos_synchronization {

class FreeRTOSMutex {
  public:
    FreeRTOSMutex() { handle = xSemaphoreCreateMutexStatic(&static_data); }
    FreeRTOSMutex(const FreeRTOSMutex &) = delete;
    FreeRTOSMutex(const FreeRTOSMutex &&) = delete;
    auto operator=(const FreeRTOSMutex &) -> FreeRTOSMutex & = delete;
    auto operator=(const FreeRTOSMutex &&) -> FreeRTOSMutex && = delete;

    ~FreeRTOSMutex() { vSemaphoreDelete(handle); }

    auto acquire() -> bool {
        return xSemaphoreTake(handle, portMAX_DELAY) == pdTRUE;
    }

    auto release() -> bool { return xSemaphoreGive(handle) == pdTRUE; }

    auto get_count() -> int { return uxSemaphoreGetCount(handle); }

  private:
    SemaphoreHandle_t handle{};
    StaticSemaphore_t static_data{};
};

class FreeRTOSMutexFromISR {
  public:
    FreeRTOSMutexFromISR() {
        handle = xSemaphoreCreateMutexStatic(&static_data);
    }
    FreeRTOSMutexFromISR(const FreeRTOSMutexFromISR &) = delete;
    FreeRTOSMutexFromISR(const FreeRTOSMutexFromISR &&) = delete;
    auto operator=(const FreeRTOSMutexFromISR &)
        -> FreeRTOSMutexFromISR & = delete;
    auto operator=(const FreeRTOSMutexFromISR &&)
        -> FreeRTOSMutexFromISR && = delete;

    ~FreeRTOSMutexFromISR() { vSemaphoreDelete(handle); }

    void acquire() {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreTakeFromISR(handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            taskYIELD();
        }
    }

    void release() {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            taskYIELD();
        }
    }

    auto get_count() -> int { return uxSemaphoreGetCount(handle); }

  private:
    SemaphoreHandle_t handle{};
    StaticSemaphore_t static_data{};
};

class FreeRTOSCriticalSection {
  public:
    FreeRTOSCriticalSection() = default;
    FreeRTOSCriticalSection(const FreeRTOSCriticalSection &) = delete;
    FreeRTOSCriticalSection(FreeRTOSCriticalSection &&) = delete;
    auto operator=(const FreeRTOSCriticalSection &)
        -> FreeRTOSCriticalSection & = delete;
    auto operator=(FreeRTOSCriticalSection &&)
        -> FreeRTOSCriticalSection && = delete;
    ~FreeRTOSCriticalSection() = default;

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void acquire() { taskENTER_CRITICAL(); }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void release() { taskEXIT_CRITICAL(); }
};

class FreeRTOSCriticalSectionRAII {
  public:
    FreeRTOSCriticalSectionRAII() { taskENTER_CRITICAL(); }
    FreeRTOSCriticalSectionRAII(const FreeRTOSCriticalSectionRAII &) = delete;
    FreeRTOSCriticalSectionRAII(FreeRTOSCriticalSectionRAII &&) = delete;
    auto operator=(const FreeRTOSCriticalSectionRAII &)
        -> FreeRTOSCriticalSectionRAII & = delete;
    auto operator=(FreeRTOSCriticalSectionRAII &&)
        -> FreeRTOSCriticalSectionRAII && = delete;
    ~FreeRTOSCriticalSectionRAII() { taskEXIT_CRITICAL(); }
};

}  // namespace freertos_synchronization
