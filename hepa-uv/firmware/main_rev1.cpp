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
#include "hepa-uv/core/messages.hpp"
#include "hepa-uv/core/tasks.hpp"
#include "hepa-uv/firmware/hepa_control_hardware.hpp"
#include "hepa-uv/firmware/hepa_hardware.h"
#include "hepa-uv/firmware/led_control_hardware.hpp"
#include "hepa-uv/firmware/utility_gpio.h"
#include "hepa-uv/firmware/uv_control_hardware.hpp"
#include "hepa-uv/firmware/uv_hardware.h"
#include "i2c/firmware/i2c_comms.hpp"
#include "timer_hardware.h"
#include "uv_hardware.h"

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
static auto i2c_handles = I2CHandlerStruct{};

// The address for the eeprom on rev a is 0x50
#if PCBA_PRIMARY_REVISION == 'a'
static constexpr uint16_t eeprom_i2c_addr = 0x50;
#else
static constexpr uint16_t eeprom_i2c_addr = 0x51;
#endif

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(
              eeprom::hardware_iface::EEPromChipType::ST_M24128_BF,
              eeprom_i2c_addr) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hw_iface = EEPromHardwareInterface();

static auto gpio_drive_pins = gpio_drive_hardware::GpioDrivePins{
    .door_open =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = DOOR_OPEN_MCU_PORT,
            .pin = DOOR_OPEN_MCU_PIN,
            .active_setting = DOOR_OPEN_MCU_AS},
    .reed_switch =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = REED_SW_MCU_PORT,
            .pin = REED_SW_MCU_PIN,
            .active_setting = REED_SW_MCU_AS},
    .hepa_push_button =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = HEPA_NO_MCU_PORT,
            .pin = HEPA_NO_MCU_PIN,
        },
    .uv_push_button =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = nUV_PRESSED_PORT,
            .pin = nUV_PRESSED_PIN,
        },
    .hepa_on_off =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = HEPA_ON_OFF_PORT,
            .pin = HEPA_ON_OFF_PIN,
            .active_setting = HEPA_ON_OFF_AS},
    .uv_on_off = gpio::PinConfig{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = UV_ON_OFF_MCU_PORT,
        .pin = UV_ON_OFF_MCU_PIN,
        .active_setting = UV_ON_OFF_AS},
    .safety_relay_active = gpio::PinConfig{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = nSAFETY_ACTIVE_MCU_PORT,
        .pin = nSAFETY_ACTIVE_MCU_PIN,
        .active_setting = nSAFETY_ACTIVE_AS},
    };

static auto& hepauv_queues = hepauv_tasks::get_main_queues();

static auto led_hardware = led_control_hardware::LEDControlHardware();
static auto hepa_hardware = hepa_control_hardware::HepaControlHardware();
static auto uv_hardware = uv_control_hardware::UVControlHardware();

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case DOOR_OPEN_MCU_PIN:
        case REED_SW_MCU_PIN:
        case HEPA_NO_MCU_PIN:
        case nUV_PRESSED_PIN:
            if (hepauv_queues.hepa_queue != nullptr) {
                static_cast<void>(hepauv_queues.hepa_queue->try_write_isr(
                    GPIOInterruptChanged{.pin = GPIO_Pin}));
            }
            if (hepauv_queues.uv_queue != nullptr) {
                static_cast<void>(hepauv_queues.uv_queue->try_write_isr(
                    GPIOInterruptChanged{.pin = GPIO_Pin}));
            }
            break;
        default:
            break;
    }
}

extern "C" void hepa_fan_rpm_callback(uint16_t rpm) {
    hepa_hardware.hepa_fan_rpm_irq(rpm);
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();
    initialize_pwm_hardware();
    initialize_adc_hardware();
    initialize_tachometer(&hepa_fan_rpm_callback);
    // set push button leds white
    led_hardware.set_button_led_power(HEPA_BUTTON, 0, 0, 0, 50);
    led_hardware.set_button_led_power(UV_BUTTON, 0, 0, 0, 50);

    app_update_clear_flags();

    i2c_setup(&i2c_handles);
    i2c_comms2.set_handle(i2c_handles.i2c2);

    canbus.start(can_bit_timings);

    hepauv_tasks::start_tasks(canbus, gpio_drive_pins, hepa_hardware,
                              uv_hardware, led_hardware, i2c_comms2,
                              eeprom_hw_iface);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
