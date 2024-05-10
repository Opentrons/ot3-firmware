#include <signal.h>

#include "FreeRTOS.h"
#include "gantry/core/axis_type.h"
#include "gantry/core/interfaces_proto.hpp"
#include "gantry/core/tasks_proto.hpp"
#include "gantry/simulator/interfaces.hpp"
#include "task.h"

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);

    LOG_INIT(
        get_axis_type() == GantryAxisType::gantry_x ? "GANTRY-X" : "GANTRY-Y",
        []() -> const char* {
            return pcTaskGetName(xTaskGetCurrentTaskHandle());
        });

    interfaces::initialize();
    interfaces::initialize_sim(argc, argv);

    gantry::tasks::start_tasks(
        interfaces::get_can_bus(), interfaces::get_motor().motion_controller,
        interfaces::get_spi(), interfaces::get_driver_config(),
        interfaces::get_motor_hardware_task(), *interfaces::get_sim_i2c2(),
        *interfaces::get_sim_eeprom());

    vTaskStartScheduler();
}
