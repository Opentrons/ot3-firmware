#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/clocking.h"
#include "pipettes/firmware/can_task.hpp"

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();

    auto can_reader_task = can_task::start_reader(can_bus_1);
    auto can_writer_task = can_task::start_writer(can_bus_1);

    vTaskStartScheduler();
}
