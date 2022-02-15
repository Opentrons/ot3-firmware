#pragma once

#include "common/core/adc_channel.hpp"

namespace adc {

template <int FULLSCALE_VOLTAGE, int FULLSCALE_READING>
class SimADCChannel
    : public adc::BaseADCChannel<FULLSCALE_VOLTAGE, FULLSCALE_READING> {
    auto sim_set_voltage(millivolts_t mv) { _mv_reading = mv; }
    virtual auto get_reading() -> uint16_t override {
        return static_cast<uint16_t>(_mv_reading);
    }
    millivolts_t _mv_reading = 0;
};

}  // namespace adc
