#pragma once

#include <functional>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

namespace freertos_timer {

class FreeRTOSTimer {
  public:
    /*
     * A software timer class. The priority of software timers in FreeRTOS is
     * currently set to 6. Any tasks utilizing this timer should have either the
     * same priority or higher priority than 6 for execution.
     */
    using Callback = std::function<void()>;
    FreeRTOSTimer(const char* name, Callback&& callback, uint32_t period_ms)
        : callback{std::move(callback)} {
        timer = xTimerCreateStatic(name, pdMS_TO_TICKS(period_ms), auto_reload,
                                   this, timer_callback, &timer_buffer);
    }
    auto operator=(FreeRTOSTimer&) -> FreeRTOSTimer& = delete;
    auto operator=(FreeRTOSTimer&&) -> FreeRTOSTimer&& = delete;
    FreeRTOSTimer(FreeRTOSTimer&) = delete;
    FreeRTOSTimer(FreeRTOSTimer&&) = delete;
    ~FreeRTOSTimer() { xTimerDelete(timer, 0); }

    auto is_running() -> bool { return (xTimerIsTimerActive(timer) == pdTRUE); }

    void update_callback(Callback&& new_callback) {
        callback = std::move(callback);
    }

    void update_period(uint32_t period_ms) {
        // Update the period of the timer and suppress the freertos behavior
        // that if the timer is currently stopped, changing its period activates
        // it.
        auto is_active = is_running();
        TickType_t blocking_ticks = is_active ? 1 : 0;
        xTimerChangePeriod(timer, pdMS_TO_TICKS(period_ms), blocking_ticks);
        if (!is_active) {
            stop();
            vTaskDelay(1);
        }
    }

    void start() { xTimerStart(timer, 1); }

    void stop() { xTimerStop(timer, 1); }

  private:
    TimerHandle_t timer{};
    Callback callback;
    StaticTimer_t timer_buffer{};
    UBaseType_t auto_reload = pdTRUE;

    static void timer_callback(TimerHandle_t xTimer) {
        auto* timer_id = pvTimerGetTimerID(xTimer);
        auto* instance = static_cast<FreeRTOSTimer*>(timer_id);
        instance->callback();
    }
};

}  // namespace freertos_timer
