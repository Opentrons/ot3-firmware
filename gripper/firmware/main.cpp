#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on

#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/utility_gpio.h"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    interfaces::initialize();

    gripper_tasks::start_tasks(
        interfaces::get_can_bus(), interfaces::get_z_motor().motion_controller,
        interfaces::get_z_motor().driver,
        interfaces::get_brushed_motor_driver_hardware_iface(),
        interfaces::get_brushed_motor_hardware_iface());

    vTaskStartScheduler();
}
