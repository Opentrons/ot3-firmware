#include "pipettes/firmware/utility_configurations.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto utility_configs::led_gpio(PipetteType pipette_type) -> gpio::PinConfig {
    switch (pipette_type) {
        case PipetteType::NINETY_SIX_CHANNEL:
        case PipetteType::THREE_EIGHTY_FOUR_CHANNEL:
            return gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_14,
                .active_setting = GPIO_PIN_RESET};
        case PipetteType::SINGLE_CHANNEL:
        case PipetteType::EIGHT_CHANNEL:
        default:
            return gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOB,
                .pin = GPIO_PIN_11,
                .active_setting = GPIO_PIN_RESET};
    }
}
