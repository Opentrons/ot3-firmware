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

    auto get_txrx_count() const -> std::size_t;
    auto get_last_received() const -> const std::vector<uint8_t>&;
    auto get_last_transmitted() const -> const std::vector<uint8_t>&;
    auto set_next_received(const std::vector<uint8_t>& to_receive) -> void;

  private:
    RegisterMap register_map;
    uint8_t read_register{0};
    // This status byte is unused currently but may have some use in the future.
    uint8_t status{0};
    std::size_t txrx_count = 0;
    std::vector<uint8_t> last_transmitted{};
    std::vector<uint8_t> last_received{};
    std::vector<uint8_t> next_receive{};
    static constexpr uint8_t write_mask =
        static_cast<uint8_t>(spi::hardware::Mode::WRITE);
};

}  // namespace hardware

}  // namespace spi
