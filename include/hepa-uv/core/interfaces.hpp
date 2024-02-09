#pragma once

#include "hepa-uv/core/constants.h"

namespace led_control {

class LEDControlInterface {
  public:
    LEDControlInterface() = default;
    LEDControlInterface(const LEDControlInterface&) = delete;
    LEDControlInterface(LEDControlInterface&&) = delete;
    auto operator=(LEDControlInterface&&) -> LEDControlInterface& = delete;
    auto operator=(const LEDControlInterface&) -> LEDControlInterface& = delete;
    virtual ~LEDControlInterface() = default;

    virtual auto set_button_led_power(uint8_t button, uint32_t r, uint32_t g,
                                      uint32_t b, uint32_t w) -> void = 0;
};

}  // namespace led_control

namespace hepa_control {

class HepaControlInterface {
  public:
    HepaControlInterface() = default;
    HepaControlInterface(const HepaControlInterface&) = delete;
    HepaControlInterface(HepaControlInterface&&) = delete;
    auto operator=(HepaControlInterface&&) -> HepaControlInterface& = delete;
    auto operator=(const HepaControlInterface&)
        -> HepaControlInterface& = delete;
    virtual ~HepaControlInterface() = default;

    virtual auto set_hepa_fan_speed(uint32_t duty_cycle) -> void = 0;
};
}  // namespace hepa_control