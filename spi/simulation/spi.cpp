#include <vector>

#include "spi/simulation/spi.hpp"

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"

bool spi::hardware::SimSpiDeviceBase::transmit_receive(
    const spi::utils::MaxMessageBuffer& transmit,
    spi::utils::MaxMessageBuffer& receive) {
    txrx_count++;
    uint8_t control = 0;
    uint32_t data = 0;

    last_transmitted = std::vector<uint8_t>(transmit.size());
    std::copy(transmit.begin(), transmit.end(), last_transmitted.begin());

    auto iter = transmit.begin();

    iter = bit_utils::bytes_to_int(iter, transmit.end(), control);
    iter = bit_utils::bytes_to_int(iter, transmit.end(), data);

    LOG("transmit_receive: control=%d, data=%d", control, data);

    constexpr uint8_t write_mask =
        static_cast<uint8_t>(spi::hardware::Mode::WRITE);

    auto out_iter = receive.begin();
    // Write status byte into buffer.
    out_iter = bit_utils::int_to_bytes(status, out_iter, receive.end());

    uint8_t reg = control & ~write_mask;

    if (control & write_mask) {
        // This is a write command.
        register_map[reg] = data;
    } else {
        // A read command will return the data from the previous read command.
        out_iter = bit_utils::int_to_bytes(register_map[read_register],
                                           out_iter, receive.end());
    }

    last_received = std::vector<uint8_t>(receive.size());
    std::copy(receive.begin(), receive.end(), last_transmitted.begin());
    // The register is cached for the next read operation.
    read_register = reg;
    return true;
}

auto spi::hardware::SimSpiDeviceBase::get_last_transmitted() const -> const std::vector<uint8_t> & {
    return last_transmitted;
}

auto spi::hardware::SimSpiDeviceBase::get_txrx_count() const -> std::size_t {
    return txrx_count;
}

auto spi::hardware::SimSpiDeviceBase::set_next_received(const std::vector<uint8_t> &to_receive) -> void {
    next_receive = to_receive;
}

auto spi::hardware::SimSpiDeviceBase::get_last_received() const -> const std::vector<uint8_t> & {
    return last_received;
}