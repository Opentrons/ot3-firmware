#pragma once

#include <map>

#include "common/core/spi.hpp"

namespace sim_spi {

/**
 * Implementation of TMC2130Spi protocol for simulation purposes.
 */
class SimTMC2130Spi : public spi::TMC2130Spi {
  public:
    using RegisterMap = std::map<uint8_t, uint32_t>;

    /**
     * Constructor.
     */
    SimTMC2130Spi(void) : register_map{} {}

    /**
     * Construct with a register map.
     * @param register_map map of register to value
     */
    SimTMC2130Spi(const RegisterMap& register_map)
        : register_map{register_map} {}

    void transmit_receive(const spi::TMC2130Spi::BufferType& transmit,
                          spi::TMC2130Spi::BufferType& receive);

  private:
    RegisterMap register_map;
};

}  // namespace sim_spi
