#include "common/firmware/spi_comms.hpp"
#include "motor-control/core//motion_controller.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};

static spi::Spi spi_comms(SPI_intf);

struct motion_controller::HardwareConfig PinConfigurations = {
    .direction = {.port = GPIOB,
                  .pin = GPIO_PIN_1,
                  .active_setting = GPIO_PIN_SET},
    .step = {.port = GPIOC, .pin = GPIO_PIN_8, .active_setting = GPIO_PIN_SET},
    .enable = {.port = GPIOA,
               .pin = GPIO_PIN_9,
               .active_setting = GPIO_PIN_SET},
};
