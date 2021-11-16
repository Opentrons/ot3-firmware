#include "common/firmware/spi_comms.hpp"

#include "spi.h"
#include "platform_specific_hal_conf.h"

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
 * Public Functions
 */

Spi::Spi(SPI_interface SPI_intf_instance): SPI_intf(SPI_intf_instance) {
}

void Spi::transmit_receive(const BufferType& transmit, BufferType& receive) {
    Reset_CS_Pin(SPI_intf.GPIO_handle, SPI_intf.pin);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    hal_transmit_receive(const_cast<uint8_t*>(transmit.data()), receive.data(),
                         BUFFER_SIZE, TIMEOUT, (SPI_HandleTypeDef *)this->SPI_intf.SPI_handle);
    /*
        pass GPIO_instance and pin
    */
    Set_CS_Pin(SPI_intf.GPIO_handle, SPI_intf.pin);
    
}
