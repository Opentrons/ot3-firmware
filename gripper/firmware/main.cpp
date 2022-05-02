#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on

#include "can/core/bit_timings.hpp"
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
// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<85000000, 50, 250000, 882>{};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    z_motor_iface::initialize();
    grip_motor_iface::initialize();

    can_start(can_bit_timings.clock_divider, can_bit_timings.segment_1_quanta,
              can_bit_timings.segment_2_quanta,
              can_bit_timings.max_sync_jump_width);

    gripper_tasks::start_tasks(canbus, z_motor_iface::get_z_motor(),
                               grip_motor_iface::get_grip_motor(),
                               z_motor_iface::get_spi(),
                               z_motor_iface::get_tmc2130_driver_configs());

    iWatchdog.start(6);

    vTaskStartScheduler();
}
