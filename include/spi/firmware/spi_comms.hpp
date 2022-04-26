#pragma once
#include <cstdint>

#include "spi/core/spi.hpp"
#include "spi/core/utils.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

namespace spi {

namespace hardware {

struct SPI_interface {
    SPI_HandleTypeDef* SPI_handle;
};

class Spi : public SpiDeviceBase {
  public:
    explicit Spi(SPI_interface SPI_int);

    auto transmit_receive(const utils::MaxMessageBuffer& transmit,
                          utils::MaxMessageBuffer& receive,
                          utils::ChipSelectInterface cs_intf) -> bool final;

  private:
    static constexpr uint32_t TIMEOUT = 0xFFFF;
    SPI_interface SPI_intf;
};

}  // namespace hardware

}  // namespace spi
