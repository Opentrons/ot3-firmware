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

class ADCChannel : public BaseADC::ChannelType {
  public:
    ADCChannel(ADC_HandleTypeDef* ADC_intf_instance, uint32_t channel);
    ADCChannel(const adc::ADCChannel&) = default;
    auto operator=(const adc::ADCChannel&) -> ADCChannel& = default;
    ADCChannel(adc::ADCChannel&&) noexcept = default;
    auto operator=(adc::ADCChannel&&) noexcept -> adc::ADCChannel& = default;
    ~ADCChannel() override = default;

  protected:
    auto get_reading() -> uint16_t override;
    ADC_HandleTypeDef* _hardware;

    uint32_t _hardware_channel;
};

class ADC : public BaseADC {
  public:
    ADC(ADC_HandleTypeDef* ADC_intf_instance1,
        ADC_HandleTypeDef* ADC_intf_instance2);

  protected:
    auto get_gripper_channel() -> ADCChannel& override {
        return gripper_channel;
    }
    auto get_z_channel() -> ADCChannel& override { return z_channel; }
    auto get_a_channel() -> ADCChannel& override { return a_channel; }

  private:
    ADCChannel a_channel;
    ADCChannel z_channel;
    ADCChannel gripper_channel;
};
}  // namespace adc
