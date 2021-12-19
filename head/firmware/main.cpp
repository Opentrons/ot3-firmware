#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#pragma GCC diagnostic pop

#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/clocking.h"
#include "head/firmware/can_task.hpp"

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    can_task::start_reader(can_bus_1);
    can_task::start_writer(can_bus_1);

    vTaskStartScheduler();
}
