#pragma once
#include <cstdint>
#include <span>

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

namespace adc {
struct ADC_interface {
    ADC_HandleTypeDef* ADC_handle;
};

class ADC {
  public:
    explicit ADC(ADC_interface ADC_intf);
    void get_readings(ADC_interface ADC_intf);

  private:
    ADC_interface ADC_intf;
};
}  // namespace adc
