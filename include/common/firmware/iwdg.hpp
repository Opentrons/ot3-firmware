#pragma once

#include "common/core/freertos_task.hpp"
#include "common/firmware/iwdg.h"

namespace iwdg {

struct TaskEntry {
    void operator()();
};

class IndependentWatchDog {
  public:
    IndependentWatchDog();
    IndependentWatchDog(const IndependentWatchDog&) = delete;
    IndependentWatchDog(const IndependentWatchDog&&) = delete;
    auto operator=(const IndependentWatchDog&) -> IndependentWatchDog& = delete;
    auto operator=(const IndependentWatchDog&&)
        -> IndependentWatchDog& = delete;
    ~IndependentWatchDog() = default;

    /**
     * Start the task.
     * @param priority
     */
    void start(uint32_t priority);

  private:
    TaskEntry te{};
    freertos_task::FreeRTOSTask<128, TaskEntry> task;
};

}  // namespace iwdg