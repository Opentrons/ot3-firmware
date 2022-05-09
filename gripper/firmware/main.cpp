#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"

// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "stm32g4xx_hal.h"
#pragma GCC diagnostic pop

#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"

static auto iWatchdog = iwdg::IndependentWatchDog{};
/**
 * The can bus.
 */
static auto canbus = hal_can_bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOC,
                    .pin = GPIO_PIN_6,
                    .active_setting = GPIO_PIN_RESET});
// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 85MHz assuming the CAN is clocked from PCLK1
// which has a clock divider of 2, and the system clock is 170MHZ
// - 50ns requested time quantum yields a 235ns actual
// - 250KHz bitrate requested yields 250312KHz actual
// - 88.3% sample point
// Should drive
// segment 1 = 73 quanta
// segment 2 = 11 quanta
//
// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<170 * can::bit_timings::MHZ, 100,
                                 250 * can::bit_timings::KHZ, 800>{};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    z_motor_iface::initialize();
    grip_motor_iface::initialize();

    canbus.start(can_bit_timings);
    gripper_tasks::start_tasks(canbus, z_motor_iface::get_z_motor(),
                               grip_motor_iface::get_grip_motor(),
                               z_motor_iface::get_spi(),
                               z_motor_iface::get_tmc2130_driver_configs());

    iWatchdog.start(6);

    vTaskStartScheduler();
}
