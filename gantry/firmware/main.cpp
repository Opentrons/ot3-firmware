#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on

#include "gantry/core/can_task.hpp"
#include "gantry/core/interfaces.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/clocking.h"
#include "common/firmware/utility_gpio.h"
#pragma GCC diagnostic pop

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    interfaces::initialize();
    auto can_writer = can_task::start_writer(interfaces::get_can_bus());
    auto can_reader = can_task::start_reader(interfaces::get_can_bus());

    vTaskStartScheduler();
}
