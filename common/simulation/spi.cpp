#include "common/simulation/spi.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "common/core/bit_utils.hpp"


void sim_spi::SimTMC2130Spi::transmit_receive(const BufferType& transmit, BufferType& receive) {
    uint8_t control = 0;
    uint32_t data = 0;

    auto iter = transmit.begin();

    iter = bit_utils::bytes_to_int(iter, transmit.end(), control);
    iter = bit_utils::bytes_to_int(iter, transmit.end(), data);

    constexpr uint8_t write_mask = static_cast<uint8_t>(motor_driver::Mode::WRITE);

    auto reg = control & ~write;

    if (control & write_mask) {
        register_map[reg] = data;
    }

    auto out_iter = receive.begin();
    iter = bit_utils::int_to_bytes(reg, out_iter, receive.end());
    iter = bit_utils::int_to_bytes(register_map[reg], out_iter, receive.end());
}