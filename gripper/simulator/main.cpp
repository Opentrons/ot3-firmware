#include <signal.h>

#include "FreeRTOS.h"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.\n", signum);
    exit(signum);
}

int main() {
    signal(SIGINT, signal_handler);

    interfaces::initialize();

    gripper_tasks::start_tasks(interfaces::get_can_bus(),
                               interfaces::get_motor().motion_controller,
                               interfaces::get_motor().driver);

    vTaskStartScheduler();
}
