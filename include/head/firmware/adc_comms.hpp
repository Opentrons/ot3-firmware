#pragma once
#include <cstdint>
#include <span>

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "head/firmware/adc.h"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop
#include "head/core/adc.hpp"

namespace adc {
struct ADC_interface {
    ADC_HandleTypeDef* ADC_handle;
};

class ADC : public BaseADC {
  public:
    explicit ADC(ADC_interface ADC_intf_instance1,
                 ADC_interface ADC_intf_instance2);
    auto get_readings() -> adc::RawADCReadings override;

  private:
    ADC_interface ADC_intf1;
    ADC_interface ADC_intf2;
};
}  // namespace adc