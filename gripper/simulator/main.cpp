#include <signal.h>

#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT("GRIPPER", []() -> const char* {
        return pcTaskGetName(xTaskGetCurrentTaskHandle());
    });

    z_motor_iface::initialize();
    grip_motor_iface::initialize();
    gripper_tasks::start_tasks(canbus, z_motor_iface::get_z_motor(),
                               grip_motor_iface::get_grip_motor(),
                               z_motor_iface::get_spi(),
                               z_motor_iface::get_tmc2130_driver_configs());

    vTaskStartScheduler();
}
