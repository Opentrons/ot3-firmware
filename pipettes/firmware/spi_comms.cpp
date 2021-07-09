#include "firmware/common/spi_comms.hpp"

#include <array>
#include <cstdio>

#include "common/firmware/spi.h"
#include "pipettes/core/bit_utils.hpp"
#include "pipettes/firmware/motor_control.hpp"

using namespace spi;

Spi::Spi() { handle = MX_SPI2_Init(); }

void Spi::transmit_receive(BufferType& transmit, BufferType& receive) {
    //    std::array<uint8_t, BUFFER_SIZE> aRxBuffer{};
    Reset_CS_Pin();
    HAL_SPI_TransmitReceive(&handle, transmit.data(), receive.data(),
                            BUFFER_SIZE, TIMEOUT);
    Set_CS_Pin();
}

void Spi::send_command(BufferType aTxBuffer, uint32_t& data, uint8_t& status) {
    auto aRxBuffer = std::array<uint8_t, BUFFER_SIZE>{};
    auto rxiter = aRxBuffer.begin();
    rxiter = bit_utils::int_to_bytes(status, rxiter);
    bit_utils::int_to_bytes(data, rxiter);

    transmit_receive(aTxBuffer, aRxBuffer);

    status = *aRxBuffer.begin();
    std::span sp{aRxBuffer};
    bit_utils::bytes_to_int<uint32_t, uint8_t>(sp.subspan(1, 4), data);
}
