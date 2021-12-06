#include "gantry/core/interfaces.hpp"

#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/firmware/spi_comms.hpp"
#include "gantry/core/axis_type.h"
#include "motor-control/core//motion_controller.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};

static spi::Spi spi_comms(SPI_intf);

struct motion_controller::HardwareConfig motor_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_SET},
    .enable = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_9,
        .active_setting = GPIO_PIN_SET},
};

static motor_hardware::MotorHardware motor_hardware_iface(motor_pins, &htim7);

static auto canbus = hal_can_bus::HalCanBus(can_get_device_handle());

void interfaces::initialize() {
    // Initialize SPI
    if (initialize_spi(get_axis_type()) != HAL_OK) {
        Error_Handler();
    }

    // Start the can bus
    can_start();
}

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::TMC2130Spi& { return spi_comms; }

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::MotorHardwareIface& {
    return motor_hardware_iface;
}
