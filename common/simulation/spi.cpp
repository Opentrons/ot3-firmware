#include "common/simulation/spi.hpp"

#include "common/core/bit_utils.hpp"

void sim_spi::SimTMC2130Spi::transmit_receive(
    const spi::TMC2130Spi::BufferType& transmit,
    spi::TMC2130Spi::BufferType& receive) {
    uint8_t control = 0;
    uint32_t data = 0;

    auto iter = transmit.begin();

    iter = bit_utils::bytes_to_int(iter, transmit.end(), control);
    iter = bit_utils::bytes_to_int(iter, transmit.end(), data);

    constexpr uint8_t write_mask =
        static_cast<uint8_t>(spi::TMC2130Spi::Mode::WRITE);

    uint8_t reg = control & ~write_mask;

    if (control & write_mask) {
        register_map[reg] = data;
    }

    auto out_iter = receive.begin();
    out_iter = bit_utils::int_to_bytes(reg, out_iter, receive.end());
    out_iter =
        bit_utils::int_to_bytes(register_map[reg], out_iter, receive.end());
}