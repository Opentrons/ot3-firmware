#pragma once

#include <cxxabi.h>
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
    FreeRTOSTimer(const char* name, const Callback& callback, uint32_t period_ms)
        : callback{callback} {
        timer = xTimerCreateStatic(name, pdMS_TO_TICKS(period_ms), auto_reload,
                                   this, timer_callback, &timer_buffer);
    }
    auto operator=(FreeRTOSTimer&) -> FreeRTOSTimer& = delete;
    auto operator=(FreeRTOSTimer&&) -> FreeRTOSTimer&& = delete;
    FreeRTOSTimer(FreeRTOSTimer&) = delete;
    FreeRTOSTimer(FreeRTOSTimer&&) = delete;
        ~FreeRTOSTimer() {
            xTimerDelete(timer, 0); }

        auto is_running() -> bool {
            return (xTimerIsTimerActive(timer) != pdFALSE);

        }
    void update_callback(const Callback& new_callback) {
        taskENTER_CRITICAL();
        callback = new_callback;
        taskEXIT_CRITICAL();
    }

    void update_period(uint32_t period_ms) {
        // Update the period of the timer and suppress the freertos behavior
        // that if the timer is currently stopped, changing its period activates
        // it.
        auto res = xTimerChangePeriod(timer, pdMS_TO_TICKS(period_ms), 0);

    }

    void start() {
        auto res = xTimerStart(timer, 0);
    }

    void stop() {
        auto res = xTimerStop(timer, 0);
    }


  private:
    TimerHandle_t timer{};
    Callback callback;
    StaticTimer_t timer_buffer{};
    UBaseType_t auto_reload = pdTRUE;

    static void timer_callback(TimerHandle_t xTimer) {
        auto* timer_id = pvTimerGetTimerID(xTimer);
        auto* instance = static_cast<FreeRTOSTimer*>(timer_id);
        int demangle_error = 0;
        auto name = instance->callback.target_type().name();
        auto demangled = abi::__cxa_demangle(name, 0, 0, &demangle_error);
        LOG("timer %p callback, instance @ %p; loaded is %s <%s>", xTimer, instance, demangled, name);
        if (instance->callback) {
            LOG("timer %p callback I WANT TO BE FREE", xTimer);
            instance->callback();
        }
    }
};

}  // namespace freertos_timer
