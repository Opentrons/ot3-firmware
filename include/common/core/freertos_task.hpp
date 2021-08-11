#pragma once

#include <array>

#include "FreeRTOS.h"

namespace freertos_task {

template <uint32_t StackDepth, UBaseType_t Priority, typename Entry>
class FreeRTOSTask {
  public:
    FreeRTOSTask(const char* task_name, Entry entry) : entry{entry} {
        handle = xTaskCreateStatic(f, task_name, StackDepth, this, Priority,
                                   backing.data(), &static_task);
    }
    ~FreeRTOSTask() { vTaskDelete(handle); }

  private:
    static void f(void* v) {
        auto instance =
            static_cast<FreeRTOSTask<StackDepth, Priority, Entry>*>(v);
        instance->entry();
    }
    Entry entry;
    TaskHandle_t handle;
    StaticTask_t static_task;
    std::array<StackType_t, StackDepth> backing{};
};

}  // namespace freertos_task