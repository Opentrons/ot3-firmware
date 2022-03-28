#include "pipettes/core/pipette_info.hpp"
#include "pipettes/core/pipette_type.h"
#include "platform_specific_hal_conf.h"
#include <stdint>

extern "C" auto get_pipette_type() -> PipetteType { return EIGHT_CHANNEL; }

namespace pipette_info {
auto get_name() -> PipetteName { return PipetteName::P1000_MULTI; }

auto get_model() -> uint16_t { return 0; }
};  // namespace pipette_info

struct pipette_info::config gpio_pins {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .sync_in = {.port = GPIOB,
                .pin = GPIO_PIN_5,
                .active_setting = GPIO_PIN_RESET},
    .LED_drive = {.port = GPIOA,
                  .pin = GPIO_PIN_8,
                  .active_setting = GPIO_PIN_RESET},
    .limit_switch = {
        .port = GPIOC, .pin = GPIO_PIN_2, .active_setting = GPIO_PIN_SET}
};

auto spi_pins_b() -> uint8_t { return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; }

auto spi_pins_c() -> uint8_t {
    return GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8;
}

auto spi_pins_d() -> uint8_t { return 0x0; }