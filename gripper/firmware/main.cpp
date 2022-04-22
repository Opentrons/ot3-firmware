#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on

#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"

static auto iWatchdog = iwdg::IndependentWatchDog{};
/**
 * The can bus.
 */
static auto canbus = hal_can_bus::HalCanBus(can_get_device_handle());

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    z_motor_iface::initialize();
    grip_motor_iface::initialize();

    can_start();

    gripper_tasks::start_all_tasks(canbus, z_motor_iface::get_z_motor(),
                                   grip_motor_iface::get_grip_motor());

    iWatchdog.start(6);

    vTaskStartScheduler();
}
