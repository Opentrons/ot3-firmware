#pragma once

#include <array>
#include <functional>

#include "FreeRTOS.h"
#include "task.h"

namespace freertos_task {

template <uint32_t StackDepth>
struct FreeRTOSTaskControl {
    FreeRTOSTaskControl() = default;
    auto operator=(FreeRTOSTaskControl&) -> FreeRTOSTaskControl& = delete;
    auto operator=(FreeRTOSTaskControl&&) -> FreeRTOSTaskControl&& = delete;
    FreeRTOSTaskControl(FreeRTOSTaskControl&) = delete;
    FreeRTOSTaskControl(FreeRTOSTaskControl&&) = delete;
    ~FreeRTOSTaskControl() = default;

    TaskHandle_t handle{};
    StaticTask_t static_task{};
    std::array<StackType_t, StackDepth> backing{};
};

/**
 * A FreeRTOS task.
 *
 * @tparam StackDepth The size of the stack.
 * @tparam EntryPoint The entry point type. Must be a copyable callable that
 * takes no arguments.
 */
template <uint32_t StackDepth, typename EntryPoint>
class FreeRTOSTask {
  public:
    /**
     * Constructor
     * @param entry  This is the entry point to the task.
     * @param control The task control block.
     * @param priority The tasks priority to use
     * @param task_name Task name.
     */
    FreeRTOSTask(EntryPoint&& entry, FreeRTOSTaskControl<StackDepth>& control,
                 UBaseType_t priority, const char* task_name)
        : entry{std::move(entry)}, control{control} {
        start(priority, task_name);
    }

    /**
     * Constructor
     * @param entry  This is the entry point to the task.
     * @param control The task control block.
     * @param priority The tasks priority to use
     * @param task_name Task name.
     */
    FreeRTOSTask(EntryPoint& entry, FreeRTOSTaskControl<StackDepth>& control,
                 UBaseType_t priority, const char* task_name)
        : entry{entry}, control{control} {
        start(priority, task_name);
    }
    FreeRTOSTask(FreeRTOSTask&) noexcept = default;
    FreeRTOSTask(FreeRTOSTask&&) noexcept = default;
    auto operator=(const FreeRTOSTask&) noexcept -> FreeRTOSTask& = default;
    auto operator=(FreeRTOSTask&&) noexcept -> FreeRTOSTask& = default;
    ~FreeRTOSTask() { vTaskDelete(control.handle); }

    /**
     * Get the control block.
     */
    [[nodiscard]] auto get_control() const -> FreeRTOSTaskControl<StackDepth>& {
        return control;
    }

    /**
     * Get a reference to the the entry point
     */
    [[nodiscard]] auto get_entry_point() const -> EntryPoint& { return entry; }

  private:
    void start(UBaseType_t priority, const char* task_name) {
        control.handle = xTaskCreateStatic(
            f, task_name, control.backing.size(), this, priority,
            control.backing.data(), &control.static_task);
    }

    static void f(void* v) {
        auto instance = static_cast<FreeRTOSTask<StackDepth, EntryPoint>*>(v);
        instance->entry();
    }
    EntryPoint entry;
    FreeRTOSTaskControl<StackDepth>& control;
};

}  // namespace freertos_task
