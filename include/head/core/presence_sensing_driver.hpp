#pragma once

#include <array>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"
#include "head/core/attached_tools.hpp"

namespace presence_sensing_driver {
using namespace can_ids;

class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(adc::BaseADC& adc)
        : PresenceSensingDriver(adc, attached_tools::AttachedTools{}) {}
    PresenceSensingDriver(adc::BaseADC& adc,
                          attached_tools::AttachedTools current_tools)
        : adc_comms(adc), current_tools(current_tools) {}
    auto get_readings() -> adc::MillivoltsReadings {
        return adc_comms.get_voltages();
    }
    auto get_current_tools() -> attached_tools::AttachedTools {
        return this->current_tools;
    }

    void set_current_tools(attached_tools::AttachedTools tools) {
        this->current_tools = tools;
    }

  private:
    adc::BaseADC& adc_comms;
    attached_tools::AttachedTools current_tools;
};

}  // namespace presence_sensing_driver
