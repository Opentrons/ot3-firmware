#pragma once

#include "common/core/adc_channel.hpp"

namespace adc {
template <int FULLSCALE_VOLTAGE = 1, int FULLSCALE_READING = 1>
class MockADCChannel
    : public BaseADCChannel<FULLSCALE_VOLTAGE, FULLSCALE_READING> {
  public:
    explicit MockADCChannel(uint16_t initial_value) : _value(initial_value) {}
    auto mock_set_reading(uint16_t new_value) -> void { _value = new_value; }
    auto get_reading() -> uint16_t override final { return _value; }
    uint16_t _value;
};
};  // namespace adc
