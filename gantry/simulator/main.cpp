#include "FreeRTOS.h"
#include "gantry/core/interfaces.hpp"
#include "gantry/core/tasks.hpp"
#include "task.h"

int main() {
    interfaces::initialize();

    gantry_tasks::start_tasks(interfaces::get_can_bus(),
                              interfaces::get_motor().motion_controller,
                              interfaces::get_motor().driver);

    vTaskStartScheduler();
}
