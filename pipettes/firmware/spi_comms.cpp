#include "firmware/common/spi_comms.hpp"

#include <array>
#include <cstdio>

#include "common/core/bit_utils.hpp"
#include "common/firmware/spi.h"
#include "pipettes/firmware/motor_control.hpp"

using namespace spi;

/*
 * SPI class and namespace.
 * Private:
 * transmit_receive takes a reference to the transmit and receive
 * buffers. It will then initiate a SPI transmit/receive call.
 *
 * Global Private:
 * BUFFER_SIZE -> 5
 * TIMEOUT -> 0xFFFF
 *
 * Public:
 * send_command takes a transmit buffer (with the command and data
 * already formatted), a uint8_t empty data which will be modified by
 * the information received from the stepper driver and status which will also
 * be modified by information received from the driver.
 */

/*
 * Private Functions
 */
void Spi::transmit_receive(const BufferType& transmit, BufferType& receive) {
    Reset_CS_Pin();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    HAL_SPI_TransmitReceive(&handle, const_cast<uint8_t*>(transmit.data()),
                            receive.data(), BUFFER_SIZE, TIMEOUT);
    Set_CS_Pin();
}

/*
 * Public Functions
 */

Spi::Spi() : handle(MX_SPI2_Init()){};

void Spi::send_command(const BufferType& aTxBuffer, uint32_t& data,
                       uint8_t& status) {
    auto aRxBuffer = std::array<uint8_t, BUFFER_SIZE>{};
    auto* rxiter = aRxBuffer.begin();
    rxiter = bit_utils::int_to_bytes(status, rxiter);
    bit_utils::int_to_bytes(data, rxiter);

    transmit_receive(aTxBuffer, aRxBuffer);

    status = *aRxBuffer.begin();
    std::span sp{aRxBuffer};
    bit_utils::bytes_to_int<uint32_t, uint8_t>(sp.subspan(1, 4), data);
}
