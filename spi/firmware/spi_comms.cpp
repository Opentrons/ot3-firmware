#include "spi/firmware/spi_comms.hpp"

#include "platform_specific_hal_conf.h"
#include "spi/core/utils.hpp"
#include "spi/firmware/spi.h"

using namespace spi::utils;
using namespace spi::hardware;

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
 * Public Functions
 */

Spi::Spi(SPI_interface SPI_intf_instance) : SPI_intf(SPI_intf_instance) {}

auto Spi::transmit_receive(const MaxMessageBuffer& transmit,
                           MaxMessageBuffer& receive) -> bool {
    Reset_CS_Pin(SPI_intf.GPIO_handle, SPI_intf.pin);
    auto response = hal_transmit_receive(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<uint8_t*>(transmit.data()), receive.data(), MAX_BUFFER_SIZE,
        TIMEOUT, this->SPI_intf.SPI_handle);

    Set_CS_Pin(SPI_intf.GPIO_handle, SPI_intf.pin);
    return response == HAL_OK;
}
