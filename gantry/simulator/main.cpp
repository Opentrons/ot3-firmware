#include <signal.h>

#include "FreeRTOS.h"
#include "gantry/core/axis_type.h"
#include "gantry/core/interfaces_proto.hpp"
#include "gantry/core/tasks_proto.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT(
        get_axis_type() == GantryAxisType::gantry_x ? "GANTRY-X" : "GANTRY-Y",
        []() -> const char* {
            return pcTaskGetName(xTaskGetCurrentTaskHandle());
        });

    interfaces::initialize();

    gantry::tasks::start_tasks(
        interfaces::get_can_bus(), interfaces::get_motor().motion_controller,
        interfaces::get_spi(), interfaces::get_driver_config());

    vTaskStartScheduler();
}
