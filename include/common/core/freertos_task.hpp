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
    FreeRTOSTask(const char* task_name, EntryPoint entry)
        : entry{std::move(entry)} {
        handle = xTaskCreateStatic(f, task_name, StackDepth, this, Priority,
                                   backing.data(), &static_task);
    }
    auto operator=(FreeRTOSTask&) -> FreeRTOSTask& = delete;
    auto operator=(FreeRTOSTask&&) -> FreeRTOSTask&& = delete;
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
