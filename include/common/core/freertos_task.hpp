#pragma once

#include <array>
#include <functional>

#include "FreeRTOS.h"
#include "task.h"

namespace freertos_task {

/**
 * A FreeRTOS task.
 *
 * @tparam StackDepth The size of the stack.
 * @tparam EntryPoint The entry point type. Must be a callable that
 * takes one argument.
 * @tparam TaskData Type of argument passed to EntryPoint
 */
template <uint32_t StackDepth, typename EntryPoint, typename TaskData>
class FreeRTOSTask {
  public:
    FreeRTOSTask(EntryPoint& entry_point) : entry_point{entry_point} {}
    auto operator=(FreeRTOSTask&) -> FreeRTOSTask& = delete;
    auto operator=(FreeRTOSTask&&) -> FreeRTOSTask&& = delete;
    FreeRTOSTask(FreeRTOSTask&) = delete;
    FreeRTOSTask(FreeRTOSTask&&) = delete;
    ~FreeRTOSTask() = default;

    /**
     * Constructor
     * @param arg Data passed into task entry point.
     * @param priority The tasks priority to use
     * @param task_name Task name.
     */
    void start(TaskData* arg, UBaseType_t priority, const char* task_name) {
        instance_data.first = this;
        instance_data.second = arg;
        handle = xTaskCreateStatic(f, task_name, backing.size(), &instance_data,
                                   priority, backing.data(), &static_task);
    }

  private:
    /**
     * Entry point passed to the task.
     * @param v Instance data.
     */
    static void f(void* v) {
        auto instance_data = static_cast<InstanceDataType*>(v);
        // Extract the "this" pointer.
        auto instance =
            static_cast<FreeRTOSTask<StackDepth, EntryPoint, TaskData>*>(
                instance_data->first);
        // Call the entry point with the argument
        instance->entry_point(instance_data->second);
    }

    TaskHandle_t handle{nullptr};
    StaticTask_t static_task{};
    std::array<StackType_t, StackDepth> backing{};
    EntryPoint& entry_point;
    using InstanceDataType = std::pair<void*, TaskData*>;
    InstanceDataType instance_data{nullptr, nullptr};
};

}  // namespace freertos_task
