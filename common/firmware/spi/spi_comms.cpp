#include "common/firmware/spi_comms.hpp"

#include "common/firmware/spi.h"
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
struct module z_motor;
Spi::Spi() { SPI_init(); }
Spi::Spi(struct module mod) {
    static struct module tmp;
    this->SPI_config = &tmp;
    Spi::set_SPI_config(mod.comp, mod.sub_comp, mod.intf, mod.ptr);
    SPI_init();
}

SPI_HandleTypeDef MX_SPI_Init(spi::SPI_interface intf, SPI_HandleTypeDef* tmp) {
    static SPI_HandleTypeDef hspi;
    switch (intf) {
        case spi::_SPI1:
            /* SPI parameter configuration*/
            __HAL_RCC_SPI1_CLK_ENABLE();
            hspi = {.Instance = SPI1,
                    .Init = {.Mode = SPI_MODE_MASTER,
                             .Direction = SPI_DIRECTION_2LINES,
                             .DataSize = SPI_DATASIZE_8BIT,
                             .CLKPolarity = SPI_POLARITY_HIGH,
                             .CLKPhase = SPI_PHASE_2EDGE,
                             .NSS = SPI_NSS_SOFT,
                             .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                             .FirstBit = SPI_FIRSTBIT_MSB,
                             .TIMode = SPI_TIMODE_DISABLE,
                             .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                             .CRCPolynomial = 7,
                             .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                             .NSSPMode = SPI_NSS_PULSE_DISABLE}

            };

            if (HAL_SPI_Init(&hspi) != HAL_OK) {
                // Error_Handler();
            }
            break;
        case spi::_SPI2:
            /* SPI parameter configuration*/
            __HAL_RCC_SPI2_CLK_ENABLE();
            hspi = {.Instance = SPI2,
                    .Init = {.Mode = SPI_MODE_MASTER,
                             .Direction = SPI_DIRECTION_2LINES,
                             .DataSize = SPI_DATASIZE_8BIT,
                             .CLKPolarity = SPI_POLARITY_HIGH,
                             .CLKPhase = SPI_PHASE_2EDGE,
                             .NSS = SPI_NSS_SOFT,
                             .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                             .FirstBit = SPI_FIRSTBIT_MSB,
                             .TIMode = SPI_TIMODE_DISABLE,
                             .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                             .CRCPolynomial = 7,
                             .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                             .NSSPMode = SPI_NSS_PULSE_DISABLE}

            };

            if (HAL_SPI_Init(&hspi) != HAL_OK) {
                // Error_Handler();
            }
            break;
        case spi::_SPI3:
            /* SPI parameter configuration*/
            __HAL_RCC_SPI3_CLK_ENABLE();
            hspi = {.Instance = SPI3,
                    .Init = {.Mode = SPI_MODE_MASTER,
                             .Direction = SPI_DIRECTION_2LINES,
                             .DataSize = SPI_DATASIZE_8BIT,
                             .CLKPolarity = SPI_POLARITY_HIGH,
                             .CLKPhase = SPI_PHASE_2EDGE,
                             .NSS = SPI_NSS_SOFT,
                             .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
                             .FirstBit = SPI_FIRSTBIT_MSB,
                             .TIMode = SPI_TIMODE_DISABLE,
                             .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
                             .CRCPolynomial = 7,
                             .CRCLength = SPI_CRC_LENGTH_DATASIZE,
                             .NSSPMode = SPI_NSS_PULSE_DISABLE}

            };

            if (HAL_SPI_Init(&hspi) != HAL_OK) {
                // Error_Handler();
            }
            break;
    }
    // tmp=&hspi;
    return hspi;
}
void Spi::set_SPI_config(component comp, sub_component sub_comp,
                         SPI_interface intf, void* SPI_HandleTypeDef_instance) {
    if (comp == head && sub_comp == motor_z) {
        // null check??
        SPI_HandleTypeDef* tmp;
        SPI_HandleTypeDef tmp2 = MX_SPI_Init(intf, tmp);
        this->SPI_config->ptr = &tmp2;
        this->SPI_config->comp = comp;
        this->SPI_config->sub_comp = sub_comp;
        this->SPI_config->intf = intf;
    }
}
void Spi::transmit_receive(const BufferType& transmit, BufferType& receive) {
    Reset_CS_Pin();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    hal_transmit_receive(const_cast<uint8_t*>(transmit.data()), receive.data(),
                         BUFFER_SIZE, TIMEOUT);
    Set_CS_Pin();
}