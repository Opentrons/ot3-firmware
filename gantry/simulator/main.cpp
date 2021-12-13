#include "FreeRTOS.h"
#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"
#include "task.h"

int main() {
    interfaces::initialize();
    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    auto can_reader = can_task::start_reader(interfaces::get_can_bus());

    vTaskStartScheduler();
}
