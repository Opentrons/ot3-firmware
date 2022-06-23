#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on

#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/utility_gpio.h"
#include "gantry/core/interfaces_rev1.hpp"
#include "gantry/core/tasks_rev1.hpp"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    interfaces::initialize();

    gantry::tasks::start_tasks(
        interfaces::get_can_bus(), interfaces::get_motor().motion_controller,
        interfaces::get_spi(), interfaces::get_driver_config());

    vTaskStartScheduler();
}
