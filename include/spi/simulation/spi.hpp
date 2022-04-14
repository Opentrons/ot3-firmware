#pragma once

#include <map>

#include "spi/core/spi.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace hardware {
/**
 * Implementation of SpiDeviceBase protocol for simulation purposes.
 */
class SimSpiDeviceBase : public SpiDeviceBase {
  public:
    using RegisterMap = std::map<uint8_t, uint32_t>;

    /**
     * Constructor.
     */
    SimSpiDeviceBase(void) : register_map{} {}

    /**
     * Construct with a register map.
     * @param register_map map of register to value
     */
    SimSpiDeviceBase(const RegisterMap& register_map)
        : register_map{register_map} {}

    bool transmit_receive(const spi::utils::MaxMessageBuffer& transmit,
                          spi::utils::MaxMessageBuffer& receive) final;

  private:
    RegisterMap register_map;
    uint8_t read_register{0};
    // This status byte is unused currently but may have some use in the future.
    uint8_t status{0};
};

}  // namespace hardware

}  // namespace spi
