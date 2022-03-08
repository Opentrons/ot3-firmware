#include "FreeRTOS.h"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "task.h"

int main() {
    interfaces::initialize();

    gripper_tasks::start_tasks(interfaces::get_can_bus(),
                               interfaces::get_motor().motion_controller,
                               interfaces::get_motor().driver);

    vTaskStartScheduler();
}
