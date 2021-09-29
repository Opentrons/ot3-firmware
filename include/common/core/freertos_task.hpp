#pragma once

#include <array>
#include <functional>

#include "FreeRTOS.h"
#include "task.h"

namespace freertos_task {

template <uint32_t StackDepth, UBaseType_t Priority>
class FreeRTOSTask {
  public:
    using EntryPoint = std::function<void()>;
    FreeRTOSTask(const char* task_name, const EntryPoint entry) : entry{entry} {
        handle = xTaskCreateStatic(f, task_name, StackDepth, this, Priority,
                                   backing.data(), &static_task);
    }
    FreeRTOSTask& operator=(FreeRTOSTask&) = delete;
    FreeRTOSTask&& operator=(FreeRTOSTask&&) = delete;
    FreeRTOSTask(FreeRTOSTask&) = delete;
    FreeRTOSTask(FreeRTOSTask&&) = delete;

    ~FreeRTOSTask() { vTaskDelete(handle); }

  private:
    static void f(void* v) {
        auto instance = static_cast<FreeRTOSTask<StackDepth, Priority>*>(v);
        instance->entry();
    }
    const EntryPoint entry;
    TaskHandle_t handle{};
    StaticTask_t static_task{};
    std::array<StackType_t, StackDepth> backing{};
};

}  // namespace freertos_task