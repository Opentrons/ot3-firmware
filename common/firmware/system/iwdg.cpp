#include "common/firmware/iwdg.h"

#include "common/firmware/iwdg.hpp"

/** How long to sleep between refreshes of watchdog */
auto SLEEP_TIME = ((IWDG_INTERVAL_MS - 50) * portTICK_PERIOD_MS);

void iwdg::TaskEntry::operator()() {
    for (;;) {
        iwdg_refresh();
        vTaskDelay(SLEEP_TIME);
    }
}

iwdg::IndependentWatchDog::IndependentWatchDog() : task{te} { MX_IWDG_Init(); }

void iwdg::IndependentWatchDog::start(uint32_t priority) {
    task.start(priority, "iwdg");
}
