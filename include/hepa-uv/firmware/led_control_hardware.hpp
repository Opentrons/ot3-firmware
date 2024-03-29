#pragma once

#include "hepa-uv/core/interfaces.hpp"

namespace led_control_hardware {

class LEDControlHardware : public led_control::LEDControlInterface {
  public:
    LEDControlHardware() = default;
    LEDControlHardware(const LEDControlHardware&) = delete;
    LEDControlHardware(LEDControlHardware&&) = delete;
    auto operator=(LEDControlHardware&&) -> LEDControlHardware& = delete;
    auto operator=(const LEDControlHardware&) -> LEDControlHardware& = delete;
    ~LEDControlHardware() final = default;

    void set_button_led_power(uint8_t button, uint32_t r, uint32_t g,
                              uint32_t b, uint32_t w) final;
};

}  // namespace led_control_hardware