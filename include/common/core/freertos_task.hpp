#pragma once

#include <array>
#include <tuple>

#include "FreeRTOS.h"
#include "task.h"

namespace freertos_task {

/**
 * A FreeRTOS task.
 *
 * @tparam StackDepth The size of the stack.
 * @tparam EntryPoint The entry point type. Must be a callable that
 * takes one argument.
 * @tparam TaskArgs Pointer types of arguments passed to EntryPoint
 */
template <uint32_t StackDepth, typename EntryPoint, typename... TaskArgs>
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
     * @param priority The tasks priority to use
     * @param task_name Task name.
     * @param task_args Data passed into task entry point.
     */
    void start(UBaseType_t priority, const char* task_name,
               TaskArgs*... task_args) {
        instance_data.first = this;
        instance_data.second = std::make_tuple(task_args...);
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
            static_cast<FreeRTOSTask<StackDepth, EntryPoint, TaskArgs...>*>(
                instance_data->first);
        // Call the entry point with the argument
        std::apply(instance->entry_point, instance_data->second);
    }

    TaskHandle_t handle{nullptr};
    StaticTask_t static_task{};
    std::array<StackType_t, StackDepth> backing{};
    EntryPoint& entry_point;
    using InstanceDataType = std::pair<void*, std::tuple<TaskArgs*...>>;
    InstanceDataType instance_data{nullptr, {}};
};

}  // namespace freertos_task
