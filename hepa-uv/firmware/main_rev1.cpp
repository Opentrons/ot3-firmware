#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
#include "hepa-uv/firmware/i2c_setup.h"
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
#include "hepa-uv/core/interfaces.hpp"
#include "hepa-uv/core/tasks.hpp"
#include "hepa-uv/firmware/utility_gpio.h"
#include "i2c/firmware/i2c_comms.hpp"
#include "sensors/firmware/sensor_hardware.hpp"

static auto iWatchdog = iwdg::IndependentWatchDog{};

/**
 * The can bus.
 */
static auto canbus = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = LED_DRIVE_PORT,
                    .pin = LED_DRIVE_PIN,
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
                                 500 * can::bit_timings::KHZ, 800>{};

/**
 * I2C handles
 */
static auto i2c_comms2 = i2c::hardware::I2C();
static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_handles = I2CHandlerStruct{};

static constexpr auto eeprom_chip =
    eeprom::hardware_iface::EEPromChipType::ST_M24128_BF;

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(eeprom_chip) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hw_iface = EEPromHardwareInterface();

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    app_update_clear_flags();

    i2c_setup(&i2c_handles);
    i2c_comms2.set_handle(i2c_handles.i2c2);
    i2c_comms3.set_handle(i2c_handles.i2c3);

    canbus.start(can_bit_timings);

    hepauv_tasks::start_tasks(
        canbus, i2c_comms2, i2c_comms3, eeprom_hw_iface);

    iWatchdog.start(6);

    vTaskStartScheduler();
}