#pragma once

#include "common/core/adc_channel.hpp"

namespace adc {
template <int FULLSCALE_VOLTAGE, int FULLSCALE_READING>
class MockADCChannel
    : public BaseADCChannel<FULLSCALE_VOLTAGE, FULLSCALE_READING> {
  public:
    MockADCChannel(uint16_t initial_mv)
        : _value(value_from_voltage(initial_mv)) {}
    auto mock_set_reading(uint16_t new_value) -> void { _value = new_value; }
    auto mock_set_reading_by_voltage(uint16_t new_mv) -> void {
        _value = value_from_voltage(new_mv);
    }
    [[nodiscard]] constexpr auto value_from_voltage(uint16_t voltage)
        -> uint16_t {
        return static_cast<uint16_t>(static_cast<float>(voltage) *
                                     static_cast<float>(FULLSCALE_READING) /
                                     static_cast<float>(FULLSCALE_VOLTAGE));
    }
    auto get_reading() -> uint16_t override final { return _value; }
    uint16_t _value;
};
};  // namespace adc
