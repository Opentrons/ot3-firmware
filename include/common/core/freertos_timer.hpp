#pragma once

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
    FreeRTOSTimer(const char* name, TimerCallbackFunction_t callback)
        : block_time(timer_period) {
        timer = xTimerCreateStatic(name, block_time, auto_reload, this,
                                   callback, &timer_buffer);
    }
    auto operator=(FreeRTOSTimer&) -> FreeRTOSTimer& = delete;
    auto operator=(FreeRTOSTimer&&) -> FreeRTOSTimer&& = delete;
    FreeRTOSTimer(FreeRTOSTimer&) = delete;
    FreeRTOSTimer(FreeRTOSTimer&&) = delete;

    void start() { xTimerStart(timer, block_time); }

    void stop() { xTimerStop(timer, block_time); }

  private:
    TimerHandle_t timer;
    TickType_t block_time;
    StaticTimer_t timer_buffer{};
    UBaseType_t auto_reload = pdTRUE;
};

}  // namespace freertos_timer
