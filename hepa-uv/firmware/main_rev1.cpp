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
#include "hepa-uv/firmware/led_control_hardware.hpp"
#include "hepa-uv/firmware/led_hardware.h"
#include "hepa-uv/firmware/hardware.h"
#include "hepa-uv/firmware/hepa_control_hardware.hpp"
#include "hepa-uv/firmware/utility_gpio.h"

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
            .port = UV_NO_MCU_PORT,
            .pin = UV_NO_MCU_PIN,
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
        .active_setting = UV_ON_OFF_AS}};

static auto& hepauv_queues = hepauv_tasks::get_main_queues();

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
        case DOOR_OPEN_MCU_PIN:
        case REED_SW_MCU_PIN:
        case HEPA_NO_MCU_PIN:
        case UV_NO_MCU_PIN:
            if (hepauv_queues.hepa_queue != nullptr) {
                static_cast<void>(hepauv_queues.hepa_queue->try_write_isr(
                    interrupt_task_messages::GPIOInterruptChanged{
                        .pin = GPIO_Pin}));
            }
            if (hepauv_queues.uv_queue != nullptr) {
                static_cast<void>(hepauv_queues.uv_queue->try_write_isr(
                    interrupt_task_messages::GPIOInterruptChanged{
                        .pin = GPIO_Pin}));
            }
            break;
        default:
            break;
    }
}

static auto led_hardware = led_control_hardware::LEDControlHardware();
static auto hepa_hardware = hepa_control_hardware::HepaControlHardware();

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();
    button_led_hw_initialize();
    initialize_hardware();

    app_update_clear_flags();

    canbus.start(can_bit_timings);

    hepauv_tasks::start_tasks(canbus, gpio_drive_pins, hepa_hardware, led_hardware);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
