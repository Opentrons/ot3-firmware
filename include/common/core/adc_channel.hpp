#pragma once

#include <concepts>
#include <cstdint>
#include <utility>

namespace adc {

using millivolts_t = uint16_t;

template <uint32_t FULLSCALE_VOLTAGE, uint32_t FULLSCALE_OUTPUT>
class BaseADCChannel {
  public:
    virtual ~BaseADCChannel() = default;
    BaseADCChannel() = default;
    BaseADCChannel(const BaseADCChannel&) = default;
    auto operator=(const BaseADCChannel&) -> BaseADCChannel& = default;
    BaseADCChannel(BaseADCChannel&&) noexcept = default;
    auto operator=(BaseADCChannel&&) noexcept -> BaseADCChannel& = default;

    auto get_voltage() -> millivolts_t {
        auto value = get_reading();
        return static_cast<millivolts_t>(
            (static_cast<float>(value) * FULLSCALE_VOLTAGE) / FULLSCALE_OUTPUT);
    }

    virtual auto get_reading() -> uint16_t = 0;
};

}  // namespace adc
