#pragma once

#include <functional>

#include "FreeRTOS.h"
#include "timers.h"

namespace freertos_timer {

// pdMS_TO_TICKS converts milliseconds to ticks. This can only be used for
// FreeRTOS tick rates less than 1000 Hz.
template <TickType_t timer_period = pdMS_TO_TICKS(2)>
class FreeRTOSTimer {
  public:
    /*
     * A software timer class. The priority of software timers in FreeRTOS is
     * currently set to 6. Any tasks utilizing this timer should have either the
     * same priority or higher priority than 6 for execution.
     */
    FreeRTOSTimer(const char* name, std::function<void()> &callback)
        : block_time(timer_period), callback(callback) {
        timer = xTimerCreateStatic(name, block_time, auto_reload, this,
                                   timer_callback, &timer_buffer);
    }
    auto operator=(FreeRTOSTimer&) -> FreeRTOSTimer& = delete;
    auto operator=(FreeRTOSTimer&&) -> FreeRTOSTimer&& = delete;
    FreeRTOSTimer(FreeRTOSTimer&) = delete;
    FreeRTOSTimer(FreeRTOSTimer&&) = delete;

    void start() { xTimerStart(timer, 0); }

    void stop() { xTimerStop(timer, 0); }

  private:
    TimerHandle_t timer{};
    TickType_t block_time;
    std::function<void()> callback;
    StaticTimer_t timer_buffer{};
    UBaseType_t auto_reload = pdTRUE;

    static void timer_callback(TimerHandle_t xTimer) {
        auto *timer_id = pvTimerGetTimerID(xTimer);
        auto instance =
            static_cast<FreeRTOSTimer<timer_period>*>(timer_id);
        instance->callback();
    }
};

}  // namespace freertos_timer
