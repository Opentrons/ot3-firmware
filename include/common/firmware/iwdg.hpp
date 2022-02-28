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