#include "common/firmware/spi_comms.hpp"

#include "common/firmware/spi.h"

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
struct module z_motor;
Spi::Spi() { SPI_init(); }
Spi::Spi(struct module mod) {
    static struct module tmp;
    this->SPI_config = &tmp;
    Spi::set_SPI_config(mod.comp, mod.sub_comp, mod.intf, mod.ptr);
    SPI_init();
}
void Spi::set_SPI_config(component comp, sub_component sub_comp, SPI_interface intf,void* SPI_HandleTypeDef_instance){
    if (comp == head && sub_comp == motor_z){
        
        //this->SPI_config->SPI_HandleTypeDef_instance=&MX_SPI3_Init();
        this->SPI_config->comp=comp;
        this->SPI_config->sub_comp=sub_comp;
        this->SPI_config->intf=intf;
    }
}
void Spi::transmit_receive(const BufferType& transmit, BufferType& receive) {
    Reset_CS_Pin();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    hal_transmit_receive(const_cast<uint8_t*>(transmit.data()), receive.data(),
                         BUFFER_SIZE, TIMEOUT);
    Set_CS_Pin();
}
