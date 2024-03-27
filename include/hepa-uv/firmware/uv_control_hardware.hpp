#pragma once

#include "hepa-uv/core/interfaces.hpp"

namespace uv_control_hardware {

class UVControlHardware : public uv_control::UVControlInterface {
  public:
    UVControlHardware() = default;
    UVControlHardware(const UVControlHardware&) = delete;
    UVControlHardware(UVControlHardware&&) = delete;
    auto operator=(UVControlHardware&&) -> UVControlHardware& = delete;
    auto operator=(const UVControlHardware&) -> UVControlHardware& = delete;
    ~UVControlHardware() final = default;

    // TODO: (ba, 2024-03-18): Now that we have a uv control interface,
    // lets migrate gpio.set/reset to it, so we can de-couple hardware
    // specific functionality from logic so we can fix simulation.

    auto get_uv_light_current() -> uint16_t final;
};

}  // namespace uv_control_hardware